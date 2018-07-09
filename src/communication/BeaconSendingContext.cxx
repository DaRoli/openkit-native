/**
* Copyright 2018 Dynatrace LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "BeaconSendingContext.h"

#include "communication/AbstractBeaconSendingState.h"
#include "communication/BeaconSendingInitialState.h"

#include "protocol/HTTPClient.h"
#include "configuration/Configuration.h"
#include "configuration/HTTPClientConfiguration.h"

using namespace communication;

const std::chrono::milliseconds BeaconSendingContext::DEFAULT_SLEEP_TIME_MILLISECONDS(std::chrono::seconds(1));

BeaconSendingContext::BeaconSendingContext(std::shared_ptr<openkit::ILogger> logger,
										   std::shared_ptr<providers::IHTTPClientProvider> httpClientProvider,
										   std::shared_ptr<providers::ITimingProvider> timingProvider,
										   std::shared_ptr<configuration::Configuration> configuration,
										   std::unique_ptr<communication::AbstractBeaconSendingState> initialState)
	: mLogger(logger)
	, mCurrentState(std::move(initialState))
	, mNextState(nullptr)
	, mIsInTerminalState(false)
	, mShutdown(false)
	, mInitSucceeded(false)
	, mConfiguration(configuration)
	, mHTTPClientProvider(httpClientProvider)
	, mTimingProvider(timingProvider)
	, mLastStatusCheckTime(0)
	, mLastOpenSessionBeaconSendTime(0)
	, mInitCountdownLatch(1)
	, mIsTimeSyncSupported(true)
	, mLastTimeSyncTime(-1)
	, mSessions()
{
}

BeaconSendingContext::BeaconSendingContext(std::shared_ptr<openkit::ILogger> logger,
										   std::shared_ptr<providers::IHTTPClientProvider> httpClientProvider,
										   std::shared_ptr<providers::ITimingProvider> timingProvider,
										   std::shared_ptr<configuration::Configuration> configuration)
	: BeaconSendingContext(logger, httpClientProvider, timingProvider, configuration, std::unique_ptr<AbstractBeaconSendingState>(new BeaconSendingInitialState()))
{	
}

void BeaconSendingContext::setNextState(std::shared_ptr<AbstractBeaconSendingState> nextState)
{
	mNextState = nextState;
}

bool BeaconSendingContext::isInTerminalState() const
{
	return mCurrentState->isTerminalState();
}

void BeaconSendingContext::executeCurrentState()
{
	mNextState = nullptr;
	mCurrentState->execute(*this);

	if (mNextState != nullptr && mNextState != mCurrentState)// mCcurrentState->execute(...) can trigger state changes
	{
		if (mLogger->isInfoEnabled())
		{
			mLogger->info("BeaconSendingContext executeCurrentState() - State change from '%s' to '%s'", mCurrentState->getStateName(), mNextState->getStateName());
		}
		mCurrentState = mNextState;
	}
}

void BeaconSendingContext::requestShutdown()
{
	mShutdown = true;
}

bool BeaconSendingContext::isShutdownRequested() const
{
	return mShutdown;
}

const std::shared_ptr<configuration::Configuration> BeaconSendingContext::getConfiguration() const
{
	return mConfiguration;
}

std::shared_ptr<providers::IHTTPClientProvider> BeaconSendingContext::getHTTPClientProvider()
{
	return mHTTPClientProvider;
}

std::shared_ptr<protocol::IHTTPClient> BeaconSendingContext::getHTTPClient()
{
	std::shared_ptr<configuration::HTTPClientConfiguration> httpClientConfig = mConfiguration->getHTTPClientConfiguration();
	return mHTTPClientProvider->createClient(mLogger, httpClientConfig);
}

int64_t BeaconSendingContext::getSendInterval() const
{
	return mConfiguration->getSendInterval();
}

void BeaconSendingContext::handleStatusResponse(std::unique_ptr<protocol::StatusResponse> response)
{
	mConfiguration->updateSettings(std::move(response));

	if (!isCaptureOn())
	{
		// capturing was turned off
		clearAllSessionData();
	}
}

void BeaconSendingContext::clearAllSessionData()
{
	// clear captured data from finished sessions
	auto finishedSessionsVec = mSessions.toStdVector();
	for (auto it = finishedSessionsVec.begin(); it != finishedSessionsVec.end(); it++)
	{
		(*it)->clearCapturedData();
	}
	mSessions.clear();
}

bool BeaconSendingContext::isCaptureOn() const
{
	return mConfiguration->isCapture();
}

std::shared_ptr<AbstractBeaconSendingState> BeaconSendingContext::getCurrentState() const
{
	return mCurrentState;
}

void BeaconSendingContext::setInitCompleted(bool success)
{
	mInitSucceeded = success;
	mInitCountdownLatch.countDown();
}

bool BeaconSendingContext::isInitialized() const
{
	return mInitSucceeded;
}

bool BeaconSendingContext::waitForInit()
{
	mInitCountdownLatch.await();
	return mInitSucceeded;
}

bool BeaconSendingContext::waitForInit(int64_t timeoutMillis)
{
	mInitCountdownLatch.await(timeoutMillis);
	return mInitSucceeded;
}

void BeaconSendingContext::sleep()
{
	mTimingProvider->sleep(DEFAULT_SLEEP_TIME_MILLISECONDS.count());
}

void BeaconSendingContext::sleep(int64_t ms)
{
	mTimingProvider->sleep(ms);
}

int64_t BeaconSendingContext::getLastStatusCheckTime() const
{
	return mLastStatusCheckTime;
}

void BeaconSendingContext::setLastStatusCheckTime(int64_t lastStatusCheckTime)
{
	mLastStatusCheckTime = lastStatusCheckTime;
}

void BeaconSendingContext::disableCapture()
{
	// first disable in configuration, so no further data will get collected
	mConfiguration->disableCapture();
	clearAllSessionData();
}

int64_t BeaconSendingContext::getCurrentTimestamp() const
{
	return mTimingProvider->provideTimestampInMilliseconds();
}

int64_t BeaconSendingContext::getLastOpenSessionBeaconSendTime() const
{
	return mLastOpenSessionBeaconSendTime;
}

void BeaconSendingContext::setLastOpenSessionBeaconSendTime(int64_t timestamp)
{
	mLastOpenSessionBeaconSendTime = timestamp;
}

AbstractBeaconSendingState::StateType BeaconSendingContext::getCurrentStateType() const
{
	return mCurrentState->getStateType();
}

bool BeaconSendingContext::isTimeSyncSupported() const
{
	return mIsTimeSyncSupported;
}


void BeaconSendingContext::disableTimeSyncSupport()
{
	mIsTimeSyncSupported = false;
}

bool BeaconSendingContext::isTimeSynced() const
{
	return !mIsTimeSyncSupported || getLastTimeSyncTime() >= 0;
}

int64_t BeaconSendingContext::getLastTimeSyncTime() const
{
	return mLastTimeSyncTime;
}

void BeaconSendingContext::setLastTimeSyncTime(int64_t lastTimeSyncTime)
{
	mLastTimeSyncTime = lastTimeSyncTime;
}

void BeaconSendingContext::initializeTimeSync(int64_t clusterTimeOffset, bool isTimeSyncSupported)
{
	mTimingProvider->initialize(clusterTimeOffset, isTimeSyncSupported);
}

void BeaconSendingContext::startSession(std::shared_ptr<core::Session> session)
{
	auto sessionWrapper = std::make_shared<core::SessionWrapper>(session);
	mSessions.put(sessionWrapper);
}

void BeaconSendingContext::finishSession(std::shared_ptr<core::Session> session)
{
	std::shared_ptr<core::SessionWrapper> sessionWrapper = findSessionWrapper(session);
	if (sessionWrapper != nullptr)
	{
		sessionWrapper->finishSession();
	}
}

std::vector<std::shared_ptr<core::SessionWrapper>> BeaconSendingContext::getAllNewSessions()
{
	std::vector<std::shared_ptr<core::SessionWrapper>> newSessions;
	auto sessionWrapperList = mSessions.toStdVector();
	for (auto it = sessionWrapperList.begin(); it != sessionWrapperList.end(); it++)
	{
		auto sessionWrapper = (*it);
		if (!sessionWrapper->isBeaconConfigurationSet())
		{
			newSessions.push_back(*it);
		}
	}

	return newSessions;
}

std::vector<std::shared_ptr<core::SessionWrapper>> BeaconSendingContext::getAllOpenAndConfiguredSessions()
{
	std::vector<std::shared_ptr<core::SessionWrapper>> openSessions;
	auto sessionWrapperList = mSessions.toStdVector();
	for (auto it = sessionWrapperList.begin(); it != sessionWrapperList.end(); it++)
	{
		auto sessionWrapper = (*it);
		if (sessionWrapper->isBeaconConfigurationSet() && !sessionWrapper->isSessionFinished())
		{
			openSessions.push_back(sessionWrapper);
		}
	}

	return openSessions;
}

std::vector<std::shared_ptr<core::SessionWrapper>> BeaconSendingContext::getAllFinishedAndConfiguredSessions()
{
	std::vector<std::shared_ptr<core::SessionWrapper>> finishedSessions;
	auto sessionWrapperList = mSessions.toStdVector();
	for (auto it = sessionWrapperList.begin(); it != sessionWrapperList.end(); it++)
	{
		auto sessionWrapper = (*it);
		if (sessionWrapper->isBeaconConfigurationSet() && sessionWrapper->isSessionFinished())
		{
			finishedSessions.push_back(*it);
		}
	}

	return finishedSessions;
}

std::shared_ptr<AbstractBeaconSendingState> BeaconSendingContext::getNextState()
{
	return mNextState;
}

std::shared_ptr<core::SessionWrapper> BeaconSendingContext::findSessionWrapper(std::shared_ptr<core::Session> session)
{
	auto sessionWrapperList = mSessions.toStdVector();
	for (auto it = sessionWrapperList.begin(); it != sessionWrapperList.end(); it++)
	{
		auto sessionWrapper = (*it);
		if ( sessionWrapper->getWrappedSession() == session )
		{
			return *it;
		}
	}
	return nullptr;
}

bool BeaconSendingContext::removeSession(std::shared_ptr<core::SessionWrapper> sessionWrapper)
{
	return mSessions.remove(sessionWrapper);
}