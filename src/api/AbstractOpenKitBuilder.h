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

#ifndef _API_ABSTRACTOPENKITBUILDER_H
#define _API_ABSTRACTOPENKITBUILDER_H

#include "api/OpenKit.h"
#include "api/ILogger.h"

#include <memory>
#include <stdexcept>

namespace api
{
	class OPENKIT_EXPORT AbstractOpenKitBuilder
	{
		public:
			///
			/// Constructor
			/// @param[in] endpointURL endpoint OpenKit connects to
			/// @param[in] deviceID unique device id
			///
			AbstractOpenKitBuilder(const char* endpointURL, uint64_t deviceID);

			///
			/// Destructor
			///
			virtual ~AbstractOpenKitBuilder() {}

			///
			/// Enables verbose mode. Verbose mode is only enabled if the the default logger is used.
			/// If a custom logger is provided (by calling @ref withLogger()) debug and info log output
			/// depends on the values returned by @ref isDebugEnabled() and @ref isInfoEnabled().
			///
			///
			AbstractOpenKitBuilder& enableVerbose();

			///
			/// Sets the logger. If no logger is set the default console logger is used. For the default
			/// logger verbose mode is enabled by calling @c enableVerbose.
			/// @param[in] logger the logger
			/// @return @c this for fluent usage
			///
			AbstractOpenKitBuilder& withLogger(std::shared_ptr<ILogger> logger);

			///
			/// Builds an @c OpenKit instance
			/// @return an @c OpenKit instance
			///
			virtual std::shared_ptr<OpenKit> build() = 0;

		private:
			///
			/// Returns a logger. If no longer is set, when building the OpenKit with @ref build(),
			/// the default logger is returned.
			/// @return a logger
			///
			std::shared_ptr<ILogger> getLogger();

		private:
			/// Flag to enable INFO and DEBUG logs
			bool mVerbose;

			/// The logger used to log traces
			std::shared_ptr<ILogger> mLogger;
	};
}

#endif
