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

#ifndef _CONFIGURATION_HTTPCLIENTCONFIGURATION_H
#define _CONFIGURATION_HTTPCLIENTCONFIGURATION_H

#include "core/UTF8String.h"

namespace configuration {
	///
	/// The HTTPClientConfiguration holds all http client related settings
	///
	class HTTPClientConfiguration
	{
	public:
		///
		/// Default constructor
		/// @param[in] URL the beacon URL
		/// @param[in] serverID server id
		/// @param[in] applicationID the application id
		///
		HTTPClientConfiguration(const core::UTF8String& url, uint32_t serverID, const core::UTF8String& applicationID);


		///
		/// Returns the base url for the http client
		/// @returns the base url
		///
		const core::UTF8String& getBaseURL() const;

		///
		/// Returns the server id to be used for the http client
		/// @returns the server id
		///
		uint32_t getServerID() const;
		
		///
		/// The application id for the http client
		/// @returns the application id
		///
		const core::UTF8String& getApplicationID() const;

	private:
		/// the beacon URL
		const core::UTF8String mBaseURL;

		/// the server ID
		uint32_t mServerID;

		/// the application id
		const core::UTF8String mApplicationID;
	};

}
#endif