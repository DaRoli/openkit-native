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
#include "memory.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "caching/BeaconCache.h"

#include "providers/DefaultThreadIDProvider.h"
#include "providers/DefaultTimingProvider.h"
#include "protocol/ssl/SSLStrictTrustManager.h"
#include "providers/DefaultSessionIDProvider.h"
#include "providers/DefaultHTTPClientProvider.h"

#include "api/IRootAction.h"
#include "configuration/Configuration.h"

#include "../protocol/MockHTTPClient.h"
#include "../protocol/MockBeacon.h"
#include "MockBeaconSender.h"
#include "../providers/MockHTTPClientProvider.h"


using namespace core;

static const char APP_ID[] = "appID";
static const char APP_NAME[] = "appName";

class SessionTest : public testing::Test
{
public:
	void SetUp()
	{
		threadIDProvider = std::make_shared<providers::DefaultThreadIDProvider>();
		timingProvider = std::make_shared<providers::DefaultTimingProvider>();
		sessionIDProvider = std::make_shared<providers::DefaultSessionIDProvider>();

		std::shared_ptr<configuration::HTTPClientConfiguration> httpClientConfiguration = std::make_shared<configuration::HTTPClientConfiguration>(core::UTF8String(""), 0, core::UTF8String(""));
		mockHTTPClientProvider = std::make_shared<test::MockHTTPClientProvider>();
		mockHTTPClient = std::shared_ptr<testing::NiceMock<test::MockHTTPClient>>(new testing::NiceMock<test::MockHTTPClient>(httpClientConfiguration));

		trustManager = std::make_shared<protocol::SSLStrictTrustManager>();

		std::shared_ptr<configuration::Device> device = std::shared_ptr<configuration::Device>(new configuration::Device(core::UTF8String(""), core::UTF8String(""), core::UTF8String("")));

		configuration = std::shared_ptr<configuration::Configuration>(new configuration::Configuration(device, configuration::OpenKitType::DYNATRACE,
			core::UTF8String(APP_NAME), "", APP_ID, 0, "",
			sessionIDProvider, trustManager));
		configuration->enableCapture();

		beaconCache = std::make_shared<caching::BeaconCache>();

		mockBeaconSender = std::make_shared<testing::StrictMock<test::MockBeaconSender>>(configuration, mockHTTPClientProvider, timingProvider);
		mockBeaconStrict = std::make_shared<testing::StrictMock<test::MockBeacon>>(beaconCache, configuration, core::UTF8String(""), threadIDProvider, timingProvider);	
		mockBeaconNice = std::make_shared<testing::NiceMock<test::MockBeacon>>(beaconCache, configuration, core::UTF8String(""), threadIDProvider, timingProvider);	
	}

	void TearDown()
	{

	}
public:		
	std::shared_ptr<providers::IThreadIDProvider> threadIDProvider;
	std::shared_ptr<providers::ITimingProvider> timingProvider;
	std::shared_ptr<providers::ISessionIDProvider> sessionIDProvider;
	std::shared_ptr<providers::IHTTPClientProvider> mockHTTPClientProvider;
	std::shared_ptr<testing::NiceMock<test::MockHTTPClient>> mockHTTPClient;
	std::shared_ptr<protocol::ISSLTrustManager> trustManager;

	std::shared_ptr<configuration::Configuration> configuration;
	std::shared_ptr<caching::BeaconCache> beaconCache;

	std::shared_ptr<testing::StrictMock<test::MockBeaconSender>> mockBeaconSender;
	std::shared_ptr<testing::StrictMock<test::MockBeacon>> mockBeaconStrict;
	std::shared_ptr<testing::NiceMock<test::MockBeacon>> mockBeaconNice;
};

TEST_F(SessionTest, constructorReturnsValidDefaults)
{
    // test the constructor call
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

    // verify default values
	ASSERT_TRUE(testSession != nullptr);
	ASSERT_EQ(testSession->getEndTime(), -1);
	ASSERT_TRUE(testSession->isEmpty());
}

TEST_F(SessionTest, enterActionWithNullActionName)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

    // add/enter "null-action"
    std::shared_ptr<api::IRootAction> rootAction = testSession->enterAction(nullptr);

    // we definitely got a NullRootAction instance
    ASSERT_TRUE(rootAction->isNullObject());
}

TEST_F(SessionTest, enterActionWithEmptyActionName)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

	// add/enter "null-action"
	std::shared_ptr<api::IRootAction> rootAction = testSession->enterAction("");

	// we definitely got a NullRootAction instance
	ASSERT_TRUE(rootAction->isNullObject());
}

TEST_F(SessionTest, enterNotClosedAction)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

    // add/enter one action
    auto rootAction = testSession->enterAction("Some action");
	ASSERT_TRUE(rootAction != nullptr);
    
    // verify that (because the actions is still active) it is not in the beacon cache (thus the cache is empty)
    ASSERT_TRUE(testSession->isEmpty());
}


TEST_F(SessionTest, enterSingleAction)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

	// add/enter one action
	auto rootAction = testSession->enterAction("some action");
	rootAction->leaveAction();

	// verify that the action is closed, thus moved to the beacon cache (thus the cache is no longer empty)
	ASSERT_FALSE(testSession->isEmpty());
}

TEST_F(SessionTest, enterMultipleActions)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

	// add/enter two actions
	auto rootAction1 = testSession->enterAction("some action 1");
	rootAction1->leaveAction();
	auto rootAction2 = testSession->enterAction("some action 2");
	rootAction2->leaveAction();
	
	// verify that the actions are closed, thus moved to the beacon cache (thus the cache is no longer empty)
	ASSERT_FALSE(testSession->isEmpty());
}


TEST_F(SessionTest, enterSameActions)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);
	
	// add/enter two actions
	auto rootAction1 = testSession->enterAction("some action");
	rootAction1->leaveAction();
	auto rootAction2 = testSession->enterAction("some action");
	rootAction2->leaveAction();

	// verify that the actions are closed, thus moved to the beacon cache (thus the cache is no longer empty)
	ASSERT_FALSE(testSession->isEmpty());
	// verify that multiple actions with same name are possible
	ASSERT_NE(rootAction1, rootAction2);
}

TEST_F(SessionTest, identifyUserWithNullTagDoesNothing)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);

	// identify a "null-user" must be possible
	EXPECT_CALL(*mockBeaconStrict, identifyUser(testing::_))
		.Times(0);

	// verify the correct method is being called
	testSession->identifyUser(nullptr);
}

TEST_F(SessionTest, identifyUserWithEmptyTagDoesNothing)
{
	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);

    // identify a "null-user" must be possible
	EXPECT_CALL(*mockBeaconStrict, identifyUser(testing::_))
		.Times(0);

    // verify the correct methods being called
	testSession->identifyUser("");
}

TEST_F(SessionTest, identifySingleUser)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, identifyUser(core::UTF8String("Some user")))
		.Times(1);
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(testing::Exactly(1));

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();
	// identify a single user
	testSession->identifyUser("Some user");
}

TEST_F(SessionTest, identifyMultipleUsers)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, identifyUser(core::UTF8String("Some user")))
		.Times(1);
	EXPECT_CALL(*mockBeaconStrict, identifyUser(core::UTF8String("Some other user")))
		.Times(1);
	EXPECT_CALL(*mockBeaconStrict, identifyUser(core::UTF8String("Yet another user")))
		.Times(1);
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(testing::Exactly(1));

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();

	// identify multiple users
	testSession->identifyUser("Some user");
	testSession->identifyUser("Some other user");
	testSession->identifyUser("Yet another user");
}

TEST_F(SessionTest, identifySameUser)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, identifyUser(core::UTF8String("Some user")))
		.Times(2);
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(testing::Exactly(1));

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();

	// identify two identical users
	testSession->identifyUser("Some user");
	testSession->identifyUser("Some user");
}

TEST_F(SessionTest, reportingCrashWithNullErrorNameDoesNotReportAnything)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, reportCrash(testing::_, testing::_, testing::_))
		.Times(0);

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);

	// report a crash, passing null values
	testSession->reportCrash(nullptr, "some reason", "some stack trace");
}

TEST_F(SessionTest, reportingCrashWithEmptyErrorNameDoesNotReportAnything)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, reportCrash(testing::_, testing::_, testing::_))
		.Times(0);

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);

	// report a crash, passing null values
	testSession->reportCrash( "", "some reason", "some stack trace");
}

TEST_F(SessionTest, reportingCrashWithNullReasonAndStacktraceWorks)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, reportCrash(testing::_, testing::_, testing::_))
		.Times(1);

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);

	// report a crash, passing null values
	testSession->reportCrash( "errorName", nullptr, nullptr);
}

TEST_F(SessionTest, reportSingleCrash)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, reportCrash(testing::_, testing::_, testing::_))
		.Times(1);
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(1);

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();

	// report a single crash
	testSession->reportCrash( "error name", "error reason", "the stacktrace causing the error");
}

TEST_F(SessionTest, reportMultipleCrashes)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, reportCrash(testing::_, testing::_, testing::_))
		.Times(2);
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(1);

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();

    // report multiple crashes
	testSession->reportCrash( "error name 1", "error reason 1", "the stacktrace causing the error 1");
	testSession->reportCrash( "error name 1", "error reason 2", "the stacktrace causing the error 2");
}

TEST_F(SessionTest, reportSameCrash)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconStrict, reportCrash(testing::_, testing::_, testing::_))
		.Times(2);
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(1);

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();

    // report multiple crashes
	testSession->reportCrash( "error name", "error reason", "the stacktrace causing the error");
	testSession->reportCrash( "error name", "error reason", "the stacktrace causing the error");
}

TEST_F(SessionTest, endSession)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(testing::Exactly(1));
	EXPECT_CALL(*mockBeaconStrict, getCurrentTimestamp())
		.Times(testing::Exactly(1));
	EXPECT_CALL(*mockBeaconStrict, endSession(testing::_))
		.Times(testing::Exactly(1));
	EXPECT_CALL(*mockBeaconSender, finishSession(testing::_))
		.Times(testing::Exactly(1));

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();

	//end the session
	testSession->end();
	ASSERT_NE(testSession->getEndTime(), -1);
}

TEST_F(SessionTest, endSessionTwice)
{
	// verify the correct methods being called
	EXPECT_CALL(*mockBeaconSender, startSession(testing::_))
		.Times(testing::Exactly(1));
	EXPECT_CALL(*mockBeaconStrict, getCurrentTimestamp())
		.Times(testing::Exactly(2));
	EXPECT_CALL(*mockBeaconStrict, endSession(testing::_))
		.Times(testing::Exactly(1));
	EXPECT_CALL(*mockBeaconSender, finishSession(testing::_))
		.Times(testing::Exactly(1));

	// create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconStrict);
	testSession->startSession();

	//try to end the same session twice
	testSession->end();
	testSession->end();

	//check correct session end time
	ASSERT_NE(testSession->getEndTime(), -1);
}

/* TODO johannes.baeuerle: Port once the send beacon method exists
    @Test
    public void endSessionWithOpenRootActions() {
        // create test environment
        final SessionImpl session = new SessionImpl(logger, beaconSender, beacon);

        // end the session containing open (=not left) actions
        session.enterAction("Some action 1");
        session.enterAction("Some action 2");
        session.end();

        // mock a valid status response via the HTTPClient to be sure the beacon cache is empty
        final HTTPClient httpClient = mock(HTTPClient.class);
        final StatusResponse statusResponse = new StatusResponse("", 200);
        when(httpClient.sendBeaconRequest(isA(String.class), any(byte[].class))).thenReturn(statusResponse);
        final HTTPClientProvider clientProvider = mock(HTTPClientProvider.class);
        when(clientProvider.createClient(any(HTTPClientConfiguration.class))).thenReturn(httpClient);

        session.sendBeacon(clientProvider);
        // verify that the actions if the action is still active, it is not in the beacon cache (thus cache is empty)
        assertThat(session.isEmpty(), is(true));
    }

    @Test
    public void sendBeacon() {
        // create test environment
        final Beacon beacon = mock(Beacon.class);
        final SessionImpl session = new SessionImpl(logger, beaconSender, beacon);
        final HTTPClientProvider clientProvider = mock(HTTPClientProvider.class);

        session.sendBeacon(clientProvider);

        // verify the proper methods being called
        verify(beaconSender, times(1)).startSession(session);
        verify(beacon, times(1)).send(clientProvider);
    }
*/

TEST_F(SessionTest, clearCapturedData)
{
    // create test environment
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

    // end the session containing closed actions (moved to the beacon cache)
	auto rootAction1 = testSession->enterAction("Some action 1");
	rootAction1->leaveAction();
	auto rootAction2 = testSession->enterAction("Some action 2");
	rootAction2->leaveAction();

    // verify that the actions are closed, thus moved to the beacon cache (thus the cache is no longer empty)
	ASSERT_FALSE(testSession->isEmpty());
     
	// clear the captured data
	testSession->clearCapturedData();
    
	// verify that the cached items are cleared and the cache is empty
	ASSERT_TRUE(testSession->isEmpty());
}

TEST_F(SessionTest, aNewlyConstructedSessionIsNotEnded)
{
	//given
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);

	//when, then
	ASSERT_FALSE(testSession->isSessionEnded());
}

TEST_F(SessionTest, aSessionIsEndedIfEndIsCalled)
{
	//check that finishSession is called
	EXPECT_CALL(*mockBeaconSender, finishSession(testing::_))
		.Times(testing::Exactly(1));

	//given
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);
	testSession->end();

	// then session is ended
	ASSERT_TRUE(testSession->isSessionEnded());
}

TEST_F(SessionTest, enterActionGivesNullRootActionIfSessionIsAlreadyEnded)
{
	//check that finishSession is called
	EXPECT_CALL(*mockBeaconSender, finishSession(testing::_))
		.Times(testing::Exactly(1));

    // given
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);
	testSession->end();

    // when entering an action on already ended session
    std::shared_ptr<api::IRootAction> obtained = testSession->enterAction("Test");

    // then
	ASSERT_TRUE(obtained != nullptr);
	ASSERT_TRUE(obtained->isNullObject());
}

TEST_F(SessionTest, identifyUserDoesNothingIfSessionIsEnded)
{
	EXPECT_CALL(*mockBeaconSender, finishSession(testing::_))
		.Times(testing::Exactly(1));
	//given
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);
	testSession->end();

	// when trying to identify a user on an ended session
	testSession->identifyUser("Jane Smith");

	// then
	ASSERT_TRUE(testSession->isEmpty());
}

TEST_F(SessionTest, reportCrashDoesNothingIfSessionIsEnded)
{
	EXPECT_CALL(*mockBeaconSender, finishSession(testing::_))
		.Times(testing::Exactly(1));

	//given
	std::shared_ptr<core::Session> testSession = std::make_shared<core::Session>(mockBeaconSender, mockBeaconNice);
	testSession->end();

	// when trying to identify a user on an ended session
	testSession->reportCrash("errorName", "reason", "stacktrace");

	//then
	ASSERT_TRUE(testSession->isEmpty());
}
