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

#ifndef _API_IROOTACTION_H
#define _API_IROOTACTION_H


#include <stdint.h>
#include <memory>

namespace api
{
	class IAction;
	///
	/// This interface provides the same functionality as IAction, additionally it allows to create child actions
	///
	class IRootAction
	{
	public:

		virtual ~IRootAction() {}

		///
		/// Enters an Action with a specified name in this Session.
		/// @param[in] actionName name of the Action
		/// @returns Action instance to work with
		///
		virtual std::shared_ptr<IAction> enterAction(const char* actionName) = 0;

		///
		/// Leaves this Action.
		///
		virtual void leaveAction() = 0;

		///
		/// Returns true if this instance of IRootAction is an object using 
		/// the NullObject pattern
		/// @returns @c true if the current IRootAction is a NullObject, @c false in all other cases
		///
		virtual bool isNullObject() = 0;
	};
}
#endif