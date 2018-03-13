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

#ifndef _CORE_ACTION_H
#define _CORE_ACTION_H

#include "api/IAction.h"
#include "core/util/SynchronizedQueue.h"
#include "core/UTF8String.h"

#include <memory>
#include <atomic>

namespace protocol
{
	class Beacon;
}

namespace core
{
	///
	/// Actual implementation of the IAction interface.
	///
	class Action : public api::IAction, public std::enable_shared_from_this<core::Action>
	{
	public:

		///
		/// Create an action given a beacon  and the action name
		/// @param[in] beacon the beacon used to serialize this Action
		/// @param[in] name the name of the action
		/// @param[in] sameLevelActions actions on the same level
		///
		Action(std::shared_ptr<protocol::Beacon> beacon, const UTF8String& name, util::SynchronizedQueue<std::shared_ptr<Action>>& sameLevelActions );

		///
		/// Create an action given a beacon  and the action name
		/// @param[in] beacon the beacon used to serialize this Action
		/// @param[in] name the name of the action
		/// @param[in] parentAction parent action
		///
		Action(std::shared_ptr<protocol::Beacon> beacon, const UTF8String& name, std::shared_ptr<Action> parentAction, util::SynchronizedQueue<std::shared_ptr<Action>>& sameLevelActions);

		///
		/// Destructor
		///
		virtual ~Action() {}

		///
		/// Leaves this action if no leaveAction was already called
		/// Call @c doLeaveAction if this is the first call to @c leaveAction
		/// @returns the parent Action, or @c null if there is no parent Action
		///
		virtual std::shared_ptr<api::IAction> leaveAction();

		///
		/// Returns the action ID
		/// @returns the action ID
		///
		int32_t getID() const;

		///
		/// Returns the action name
		/// @returns the action name
		///
		const core::UTF8String& getName() const;

		///
		/// Returns the ID of the parent action
		/// @returns the ID of the parent action
		///
		int32_t getParentID() const;

		///
		/// Returns the start time of the action
		/// @returns the start time of the action
		///
		int64_t getStartTime() const;

		///
		/// Returns the end time of the action
		/// @returns the end time of the action
		///
		int64_t getEndTime() const;

		///
		/// Returns the start sequence number of the action
		/// @returns the start sequence number of the action
		///
		int32_t getStartSequenceNo() const;

		///
		/// Returns the end sequence number of the action
		/// @returns the end sequence number of the action
		///
		int32_t getEndSequenceNo()const;
	protected:
		///
		/// Return a flag if this action has been closed already
		/// @returns @c true if action was already left, @c false if action is open
		///
		bool isActionLeft() const;

	protected:

		/// parent action
		std::shared_ptr<Action> mParentAction;

		/// action end time
		std::atomic<int64_t> mEndTime;
	private:

		///
		/// Leaves this Action.
		/// Called by leaveAction only if this is the first leaveAction call on this Action
		/// @returns the parent Action, or @c null if there is no parent Action
		///
		virtual std::shared_ptr<api::IAction> doLeaveAction();

		/// beacon used for serialization
		std::shared_ptr<protocol::Beacon> mBeacon;

		///action id
		int32_t mID;

		/// action name
		const core::UTF8String& mName;

		/// action start time
		int64_t mStartTime;

		/// action start sequence number
		int32_t mStartSequenceNumber;

		/// action end sequence number
		int32_t mEndSequenceNumber;

		/// actions that reside on the same Level
		util::SynchronizedQueue<std::shared_ptr<Action>>& mSameLevelActions;
	};
}
#endif