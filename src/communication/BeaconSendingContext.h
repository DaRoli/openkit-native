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

#ifndef _COMMUNICATION_BEACONSENDINGCONTEXT_H
#define _COMMUNICATION_BEACONSENDINGCONTEXT_H

#include "core/util/CountDownLatch.h"
#include "core/util/SynchronizedQueue.h"
#include "providers/IHTTPClientProvider.h"
#include "providers/ITimingProvider.h"
#include "configuration/Configuration.h"
#include "protocol/StatusResponse.h"
#include "communication/AbstractBeaconSendingState.h"
#include "core/Session.h"


#include <atomic>
#include <memory>

namespace communication
{
	///
	/// State context for beacon sending states.
	///
	class BeaconSendingContext
	{
	public:
		///
		/// Constructor
		/// @param[in] httpClientProvider provider for HTTPClient objects
		/// @param[in] timingProvider utility class for timing related stuff
		/// @param[in] configuration general configuration options
		///
		BeaconSendingContext(std::shared_ptr<providers::IHTTPClientProvider> httpClientProvider,
			std::shared_ptr<providers::ITimingProvider> timingProvider,
			std::shared_ptr<configuration::Configuration> configuration);

		virtual ~BeaconSendingContext() {};

		///
		/// Register a state following the current state once the current state finished
		/// @param nextState instance of the  AbstractBeaconSendingState that follows after the current state
		///
		virtual void setNextState(std::shared_ptr<AbstractBeaconSendingState> nextState);

		///
		/// Return a flag if the current state of this context is a terminal state
		/// @returns @c true if the current state is a terminal state
		///
		virtual bool isInTerminalState() const;

		///
		/// Executes the current state
		///
		void executeCurrentState();

		///
		/// Request shutdown
		///
		virtual void requestShutdown();

		///
		/// Return a flag if shutdown was requested
		/// @returns @c true if shutdown was requested, @c false if not
		///
		virtual bool isShutdownRequested() const;

		///
		/// Return the currently used Configuration
		/// @return configuration isntance
		///
		const std::shared_ptr<configuration::Configuration> getConfiguration() const;

		///
		/// Returns the  HTTPClient created by the current BeaconSendingContext
		/// @returns a shared pointer to the HTTPClient created by the BeaconSendingContext
		///
		virtual std::shared_ptr<protocol::IHTTPClient> getHTTPClient();

		///
		/// Handle the status response received from the server
		/// Update the current configuration accordingly
		///
		virtual void handleStatusResponse(std::unique_ptr<protocol::StatusResponse> response);

		///
		/// Clears all session data
		///
		void clearAllSessionData();

		///
		/// Returns a flag if capturing is enabled
		/// @returns @c true if capturing is enabled, @c false if capturing is disabled
		///
		virtual bool isCaptureOn() const;

		///
		/// Complete OpenKit initialisation
		/// NOTE: This will wake up every caller waiting in the @c #waitForInit() method. 
		/// @param[in] success @c true if OpenKit was successfully initialized, @c false if it was interrupted
		///
		virtual void setInitCompleted(bool success);

		///
		/// Get a boolean indicating whether OpenKit is initialized or not.
		/// @returns @c true  if OpenKit is initialized, @c false otherwise.
		///
		bool isInitialised() const;

		///
		/// Blocking method waiting until initialisation finished
		/// @return @c true if initialisation suceeded, @c false if initialisation failed
		///
		bool waitForInit();

		///
		/// Sleep for a given amount of time
		/// @param[in] ms number of milliseconds
		///
		virtual void sleep(int64_t ms);

		///
		/// Get current timestamp
		/// @returns current timestamp
		///
		virtual int64_t getCurrentTimestamp() const;

		///
		/// Get timestamp when last status check was performed
		/// @returns timestamp of last status check
		///
		int64_t getLastStatusCheckTime() const;

		///
		/// Set timestamp when last status check was performed
		/// @param[in] lastStatusCheckTime timestamp of last status check
		///
		virtual void setLastStatusCheckTime(int64_t lastStatusCheckTime);

		///
		/// Get timestamp when open sessions were sent last
		/// @returns timestamp timestamp of last sending of open session
		///
		int64_t getLastOpenSessionBeaconSendTime() const;

		///
		/// Set timestamp when open sessions were sent last
		/// @param[in] timestamp  timestamp of last sendinf of open session
		///
		virtual void setLastOpenSessionBeaconSendTime(int64_t timestamp);

		///
		/// Returns the type of state
		/// @returns type of state as defined in AbstractBeaconSendingState
		///
		AbstractBeaconSendingState::StateType getCurrentStateType() const;

		///
		/// Gets a boolean flag indicating whether time sync is supported or not.
		/// @returns @c true if time sync is supported, @c false otherwise.
		///
		virtual bool isTimeSyncSupported() const;

		///
		/// Disables the time sync
		///
		virtual void disableTimeSyncSupport();

		///
		/// Gets a boolean flag indicating whether the time sync has been performed before
		/// @returns @c true if time sync was performed, @c false otherwise
		///
		virtual bool isTimeSynced() const;

		///
		/// Returns the timestamp when time sync was executed last time.
		/// @returns the timestamp of the last successful time sync
		///
		virtual int64_t getLastTimeSyncTime() const;

		///
		/// Set the timestamp of the last successful time sync
		/// @param[in] lastTimeSyncTime timestamp 
		///
		virtual void setLastTimeSyncTime(int64_t lastTimeSyncTime);

		///
		/// Initialize time synchronisation with cluster time
		/// @param[in] clusterTimeOffset the cluster offset
		/// @param[in] @c true if time sync is supported, otherwise @c false
		///
		virtual void initializeTimeSync(int64_t clusterTimeOffset, bool isTimeSyncSupported);

		///
		/// After a session is started add it to the open sessions list
		/// @param[in] session new session to add to open sessions list
		///
		void startSession(std::shared_ptr<core::Session> session);

		///
		/// After a session is finished move it from the open sessions to the finished sessions list
		/// @param[in] session existing session to move from open to finished session list
		///
		void finishSession(std::shared_ptr<core::Session> session);

	private:
		/// instance of AbstractBeaconSendingState with the current state
		std::shared_ptr<AbstractBeaconSendingState> mCurrentState;

		/// Flag if the current state is a terminal state
		bool mIsInTerminalState;

		/// Atomic shutdown flag
		std::atomic<bool> mShutdown;

		/// Atomic flag for successful initialisation
		std::atomic<bool> mInitSucceeded;

		/// The configuration to use
		std::shared_ptr<configuration::Configuration> mConfiguration;

		/// IHTTPClientProvider responsible for creating instances of HTTPClient
		std::shared_ptr<providers::IHTTPClientProvider> mHTTPClientProvider;

		/// TimingPRovider used by the BeaconSendingContext
		std::shared_ptr<providers::ITimingProvider> mTimingProvider;

		/// time of the last status check
		int64_t mLastStatusCheckTime;

		/// time when open sessions were last sent
		int64_t mLastOpenSessionBeaconSendTime;

		/// countdown latch used for wait-on-initialisation
		core::util::CountDownLatch mInitCountdownLatch;

		/// flag if time sync is supported
		bool mIsTimeSyncSupported;

		/// timestamp of the last time sync
		int64_t mLastTimeSyncTime;

		/// container storing all open sessions
		core::util::SynchronizedQueue<std::shared_ptr<core::Session>> mOpenSessions;

		/// container storing all finished sessions
		core::util::SynchronizedQueue<std::shared_ptr<core::Session>> mFinishedSessions;
	};
}
#endif