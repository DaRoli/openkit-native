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

#ifndef _API_DYNATRACEOPENKITBUILDER_H
#define _API_DYNATRACEOPENKITBUILDER_H

#include "AbstractOpenKitBuilder.h"

namespace api
{
	class DynatraceOpenKitBuilder : public AbstractOpenKitBuilder
	{
		public:
			///
			/// Constructor
			///
			DynatraceOpenKitBuilder();

			///
			/// Destructor
			///
			virtual ~DynatraceOpenKitBuilder() {};


			//virtual std::unique_ptr<OpenKit> build() override;
	};
}

#endif
