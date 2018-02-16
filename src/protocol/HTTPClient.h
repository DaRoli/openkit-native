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

#ifndef _PROTOCOL_HTTPCLIENT_H
#define _PROTOCOL_HTTPCLIENT_H

#include <memory>

#include "curl/curl.h"

#include "protocol/StatusResponse.h"
#include "protocol/TimeSyncResponse.h"
#include "configuration/HTTPClientConfiguration.h"

namespace protocol {
	///
	/// HTTP client which abstracts the 3 basic request types:
	/// - status check
	/// - beacon send
	/// - time sync
	///
	class HTTPClient
	{
	public:
		///
		/// Default constructor
		/// @param[in] configuration configuration parameters for the HTTPClient
		///
		HTTPClient(std::shared_ptr<configuration::HTTPClientConfiguration> configuration);

		///
		/// Destructor
		///
		~HTTPClient();

		///
		/// sends a status check request and returns a status response
		/// @returns a status response with the response data for the request
		///
		std::unique_ptr<StatusResponse> sendStatusRequest();

		///
		/// sends a beacon send request and returns a status response
		/// @returns a status response with the response data for the request
		///
		std::unique_ptr<StatusResponse> HTTPClient::sendBeaconRequest(const core::UTF8String& clientIPAddress, const void* data, size_t dataSize);

		///
		/// sends a timesync request and returns a timesync response
		/// @returns a timesync response with the response data for the request
		///
		std::unique_ptr<TimeSyncResponse> HTTPClient::sendTimeSyncRequest();

	private:

		/// the type of request sent to the server
		enum RequestType
		{
			STATUS, ///< status check request
			BEACON, ///< beacon send request
			TIMESYNC ///< time sync request
		};

		/// HTTP methods
		enum HttpMethod
		{
			GET,
			POST
		};

		/// sends a status check request and returns a status response
		std::unique_ptr<Response> sendRequestInternal(const HTTPClient::RequestType requestType, const core::UTF8String& url, const core::UTF8String& clientIPAddress, const void* inData, size_t inDataSize, const HTTPClient::HttpMethod method);

		void buildMonitorURL(core::UTF8String& monitorURL, const core::UTF8String& baseURL, const core::UTF8String& applicationID, uint32_t serverID);

		void buildTimeSyncURL(core::UTF8String& monitorURL, const core::UTF8String& baseURL);

		void appendQueryParam(core::UTF8String& url, const char* key, const char* vaalue);

		std::unique_ptr<Response> handleResponse(long httpCode, const std::string& buffer);

	private:

		/// easy handle to the CURL session
		CURL * mCurl;

		/// the server ID
		const uint32_t mServerID;

		/// URL used for status check and beacon send requests
		core::UTF8String mMonitorURL;

		/// the beacon URL
		core::UTF8String mTimeSyncURL;
	};
}

#endif