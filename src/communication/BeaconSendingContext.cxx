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
#include "configuration/HTTPClientConfiguration.h"

using namespace communication;

BeaconSendingContext::BeaconSendingContext(std::shared_ptr<providers::IHTTPClientProvider> httpClientProvider,
										   std::shared_ptr<providers::ITimingProvider> timingProvider,
										   std::shared_ptr<configuration::Configuration> configuration)
	: mCurrentState(std::unique_ptr<AbstractBeaconSendingState>(new BeaconSendingInitialState()))
	, mIsInTerminalState(false)
	, mShutdown(false)
	, mHTTPClientProvider(httpClientProvider)
	, mTimingProvider(timingProvider)
	, mConfiguration(configuration)
	, mInitCountdownLatch(1)
{	
}

void BeaconSendingContext::setNextState(std::unique_ptr<AbstractBeaconSendingState> nextState)
{
	if (nextState != nullptr)
	{
		mCurrentState = std::move(nextState);
	}
}

bool BeaconSendingContext::isInTerminalState() const
{
	return mCurrentState->isAShutdownState();
}

void BeaconSendingContext::executeCurrentState()
{
	if (mCurrentState != nullptr)
	{
		mCurrentState->execute(*this);
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

std::unique_ptr<protocol::HTTPClient> BeaconSendingContext::getHTTPClient()
{
	if (mConfiguration != nullptr && mHTTPClientProvider != nullptr)
	{
		std::shared_ptr<configuration::HTTPClientConfiguration> httpClientConfig = mConfiguration->getHTTPClientConfiguration();
		if (httpClientConfig != nullptr)
		{
			return mHTTPClientProvider->createClient(httpClientConfig);
		}
	}
	return nullptr;
}

void BeaconSendingContext::handleStatusResponse(std::unique_ptr<protocol::StatusResponse> response)
{
	if (mConfiguration != nullptr)
	{
		mConfiguration->updateSettings(std::move(response));

		if (!isCaptureOn())
		{
			// capturing was turned off
			clearAllSessionData();
		}
	}
}

void BeaconSendingContext::clearAllSessionData()
{

}

bool BeaconSendingContext::isCaptureOn() const
{
	if (mConfiguration != nullptr)
	{
		return mConfiguration->isCapture();
	}
	return false;
}

void BeaconSendingContext::setInitCompleted(bool success)
{
	mInitSuceeded = success;
	mInitCountdownLatch.countDown();
}

bool BeaconSendingContext::isInitialised() const
{
	return mInitSuceeded;
}

void BeaconSendingContext::sleep(uint64_t ms)
{
	if (mTimingProvider != nullptr)
	{
		mTimingProvider->sleep(ms);
	}
}

uint64_t BeaconSendingContext::getLastStatusCheckTime() const
{
	return mLastStatusCheckTime;
}

void BeaconSendingContext::setLastStatusCheckTime(uint64_t lastStatusCheckTime)
{
	mLastStatusCheckTime = lastStatusCheckTime;
}

uint64_t BeaconSendingContext::getCurrentTimestamp() const
{
	if (mTimingProvider != nullptr)
	{
		return mTimingProvider->provideTimestampInMilliseconds();
	}
	return 0;
}

uint64_t BeaconSendingContext::getLastOpenSessionBeaconSendTime()
{
	return mLastOpenSessionBeaconSendTime;
}

void BeaconSendingContext::setLastOpenSessionBeaconSendTime(uint64_t timestamp)
{
	mLastStatusCheckTime = timestamp;
}