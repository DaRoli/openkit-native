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

#include "BeaconSendingCaptureOnState.h"

#include <chrono>
#include <algorithm>
#include <memory>

#include "communication/BeaconSendingFlushSessionsState.h"
#include "communication/AbstractBeaconSendingState.h"
#include "communication/BeaconSendingContext.h"

#include "protocol/StatusResponse.h"

using namespace communication;

BeaconSendingCaptureOnState::BeaconSendingCaptureOnState()
	: AbstractBeaconSendingState(AbstractBeaconSendingState::StateType::BEACON_SENDING_CAPTURE_ON_STATE)
{

}

void BeaconSendingCaptureOnState::doExecute(BeaconSendingContext& context)
{
	// TODO
}

std::shared_ptr<AbstractBeaconSendingState> BeaconSendingCaptureOnState::getShutdownState()
{
	return std::shared_ptr<AbstractBeaconSendingState>(new BeaconSendingFlushSessionsState());
}
