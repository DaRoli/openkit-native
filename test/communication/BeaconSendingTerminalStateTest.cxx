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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "communication/BeaconSendingTerminalState.h"
#include "communication/AbstractBeaconSendingState.h"

#include "MockBeaconSendingContext.h"

class BeaconSendingTerminalStateTest : public testing::Test
{
public:

	void SetUp()
	{
		target = new BeaconSendingTerminalState();
	}

	void TearDown()
	{
		delete target;
	}

	BeaconSendingTerminalState* target;

	testing::StrictMock<test::MockBeaconSendingContext> mockContext;//StrictMock ensure that  all additional calls on context result in failure
};

TEST_F(BeaconSendingTerminalStateTest, isTerminalStateIsTrueForTheTerminalState)
{
	EXPECT_TRUE(target->isAShutdownState());
}

TEST_F(BeaconSendingTerminalStateTest, theShutdownStateIsAlwaysTheSameReference)
{
	BeaconSendingTerminalState* terminalState = new BeaconSendingTerminalState();
	std::shared_ptr<AbstractBeaconSendingState> obtained = terminalState->getShutdownState();

	EXPECT_TRUE(obtained != nullptr);
	EXPECT_TRUE(terminalState == obtained.get());
}

TEST_F(BeaconSendingTerminalStateTest, executeRequestsShutdown)
{
	EXPECT_CALL(mockContext, requestShutdown())
		.Times(::testing::Exactly(1));

	EXPECT_CALL(mockContext, isShutdownRequested())
		.Times(::testing::Exactly(1));

	target->execute(mockContext);
}
