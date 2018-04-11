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

#ifndef _CORE_WEBREQUESTTRACERBASE_H
#define _CORE_WEBREQUESTTRACERBASE_H

#include "api/IWebRequestTracer.h"

#include <atomic>

#include "core/UTF8String.h"

namespace protocol
{
	class Beacon;
}

namespace core
{
	///
	/// This class is returned as Action by @ref OpenKit#createSession(String) when the @ref OpenKit#shutdown()
	/// has been called before.
	///
	class WebRequestTracerBase : public api::IWebRequestTracer, public std::enable_shared_from_this<WebRequestTracerBase>
	{
	public:

		///
		/// Constructor taking an Action
		/// @param[in] beacon @ref Beacon used to serialize the WebRequestTracer
		/// @param[in] parentActionId id of the parent of the WebRequestTracer
		///
		WebRequestTracerBase(std::shared_ptr<protocol::Beacon> beacon, int32_t parentActionID);

		const char* getTag() override;

		virtual std::shared_ptr<IWebRequestTracer> setResponseCode(int32_t responseCode) override;

		virtual std::shared_ptr<IWebRequestTracer> setBytesSent(int32_t bytesSent) override;

		virtual std::shared_ptr<IWebRequestTracer> setBytesReceived(int32_t bytesReceived) override;

		virtual std::shared_ptr<IWebRequestTracer> start() override;

		virtual void stop() override;

		virtual bool isNullObject() override;

		///
		/// Returns the target URL of the web request
		/// @returns target URL of the web request
		///
		const core::UTF8String getURL() const;

		///
		/// Returns the response code of the web request
		/// @returns response code of the web request 
		///
		int32_t getResponseCode() const;

		///
		/// Returns the start time of the web request
		/// @returns start time of the web request 
		///
		int64_t getStartTime() const;

		///
		/// Returns the end time of the web request
		/// @returns end time of the web request 
		///
		int64_t getEndTime() const;

		///
		/// Returns the start sequence number
		/// @returns start sequence number of the web request 
		///
		int32_t getStartSequenceNo() const;

		///
		/// Returns the end sequence number
		/// @returns end sequence number of the web request 
		///
		int32_t getEndSequenceNo() const;

		///
		/// Returns the number of bytes sent 
		/// @returns amount of bytes sent
		///
		int32_t getBytesSent() const;

		///
		/// Returns the number of bytes received 
		/// @returns amount of bytes received
		///
		int32_t getBytesReceived() const;

		///
		/// Returns a flag if stop() has already been called
		/// @returns @c true if the web request tracer has been stopped already, @c false otherwise
		///
		bool isStopped() const;

	private:

		/// @ref Beacon used to serialize the WebRequestTracer
		std::shared_ptr<protocol::Beacon> mBeacon;

		/// @ref Action which is the parent of the WebRequestTracer, can be @c nullptr if this WebRequestTracer was
		/// initialized with a @ref RootAction
		int32_t mParentActionID;

		/// Response code of the the web request
		int32_t mResponseCode;

		/// Number of bytes sent for the web request
		int32_t mBytesSent;

		/// Number of bytes received for the web request
		int32_t mBytesReceived;

		/// web request start time
		int64_t mStartTime;

		/// web request end time
		std::atomic<int64_t> mEndTime;

		/// start sequence number
		int32_t mStartSequenceNo;
		
		/// end sequence number
		int32_t mEndSequenceNo;

		/// Dynatrace tag that has to be used for tracing the web request
		UTF8String mWebRequestTag;

	protected:
		/// The target URL of the web request
		UTF8String mURL;
	};
}

#endif