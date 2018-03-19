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

#include "configuration/BeaconCacheConfiguration.h"
#include "caching/BeaconCache.h"
#include "caching/SpaceEvictionStrategy.h"
#include "../caching/MockBeaconCache.h"

#include <memory>

using namespace configuration;
using namespace caching;

class SpaceEvictionStrategyTest : public testing::Test
{
public:
	SpaceEvictionStrategyTest()
		: mockBeaconCache()
	{
	}

	void SetUp()
	{
		mockBeaconCache = std::shared_ptr<testing::NiceMock<test::MockBeaconCache>>(new testing::NiceMock<test::MockBeaconCache>());
	}

	void TearDown()
	{
		mockBeaconCache = nullptr;
	}

	std::shared_ptr<testing::NiceMock<test::MockBeaconCache>> mockBeaconCache;
};

TEST_F(SpaceEvictionStrategyTest, theStrategyIsDisabledIfCacheSizeLowerBoundIsEqualToZero)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 0L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	// then
	ASSERT_TRUE(target.isStrategyDisabled());
}

TEST_F(SpaceEvictionStrategyTest, theStrategyIsDisabledIfCacheSizeLowerBoundIsLessThanZero)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, -1L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	// then
	ASSERT_TRUE(target.isStrategyDisabled());
}

TEST_F(SpaceEvictionStrategyTest, theStrategyIsDisabledIfCacheSizeUpperBoundIsEqualToZero)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 0L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	// then
	ASSERT_TRUE(target.isStrategyDisabled());
}

TEST_F(SpaceEvictionStrategyTest, theStrategyIsDisabledIfCacheSizeUpperBoundIsLessThanLowerBound)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 999L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	// then
	ASSERT_TRUE(target.isStrategyDisabled());
}

TEST_F(SpaceEvictionStrategyTest, shouldRunGivesTrueIfNumBytesInCacheIsGreaterThanUpperBoundLimit)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	//when
	ON_CALL(*mockBeaconCache, getNumBytesInCache())
		.WillByDefault(testing::Return(configuration->getCacheSizeUpperBound() + 1));

	// then
	ASSERT_TRUE(target.shouldRun());
}

TEST_F(SpaceEvictionStrategyTest, shouldRunGivesFalseIfNumBytesInCacheIsEqualToUpperBoundLimit)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	//when
	ON_CALL(*mockBeaconCache, getNumBytesInCache())
		.WillByDefault(testing::Return(configuration->getCacheSizeUpperBound()));

	// then
	ASSERT_FALSE(target.shouldRun());
}

TEST_F(SpaceEvictionStrategyTest, shouldRunGivesFalseIfNumBytesInCacheIsLessThanUpperBoundLimit)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	//when
	ON_CALL(*mockBeaconCache, getNumBytesInCache())
		.WillByDefault(testing::Return(configuration->getCacheSizeUpperBound() - 1));

	// then
	ASSERT_FALSE(target.shouldRun());
}

TEST_F(SpaceEvictionStrategyTest, executeEvictionLogsAMessageOnceAndReturnsIfStrategyIsDisabled)
{
	// TODO: Test logger interactions once the logger is implemented
}

TEST_F(SpaceEvictionStrategyTest, executeEvictionDoesNotLogIfStrategyIsDisabledAndInfoIsDisabledInLogger)
{
	// TODO: Test logger interactions once the logger is implemented
}

TEST_F(SpaceEvictionStrategyTest, executeEvictionCallsCacheMethodForEachBeacon)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	uint32_t callCount = 0;
	ON_CALL(*mockBeaconCache, getNumBytesInCache())
		.WillByDefault(testing::Invoke(
			[&callCount]() -> int64_t {
				// callCount 1 => 2001 for SpaceEvictionStrategy::shouldRun()
				// callCount 2 => 2001 for outer while loop in SpaceEvictionStrategy::doExecute()
				// callCount 3 => 2001 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 1
				// callCount 4 => 2001 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 42
				// callCount 5 => 0 for outer while loop in SpaceEvictionStrategy::doExecute() (to exit the while loop)
				callCount++;
				return callCount < 5 ? 2001L : 0L;
			}
		));
	ON_CALL(*mockBeaconCache, getBeaconIDs())
		.WillByDefault(testing::Return(std::unordered_set<int32_t>({ 1, 42 })));

	// then
	EXPECT_CALL(*mockBeaconCache, getNumBytesInCache())
		.Times(testing::Exactly(5));
	EXPECT_CALL(*mockBeaconCache, evictRecordsByNumber(1, 1))
		.Times(testing::Exactly(1));
	EXPECT_CALL(*mockBeaconCache, evictRecordsByNumber(42, 1))
		.Times(testing::Exactly(1));

	// when
	target.execute();
}

TEST_F(SpaceEvictionStrategyTest, executeEvictionLogsEvictionResultIfDebugIsEnabled)
{
	// TODO: Test logger interactions once the logger is implemented
}

TEST_F(SpaceEvictionStrategyTest, executeEvictionDoesNotLogEvictionResultIfDebugIsDisabled)
{
	// TODO: Test logger interactions once the logger is implemented
}

TEST_F(SpaceEvictionStrategyTest, executeEvictionRunsUntilTheCacheSizeIsLessThanOrEqualToLowerBound)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	uint32_t callCount = 0;
	ON_CALL(*mockBeaconCache, getNumBytesInCache())
		.WillByDefault(testing::Invoke(
			[&callCount]() -> int64_t {
		callCount++;
		if (callCount < 2)
		{
			// callCount 1 => 2001 for SpaceEvictionStrategy::shouldRun()
			return 2001L;
		}
		if (callCount < 5)
		{
			// callCount 2 => 2000 for outer while loop in SpaceEvictionStrategy::doExecute()
			// callCount 3 => 2000 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 1
			// callCount 4 => 2000 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 42
			return 2000L;
		}
		else if (callCount < 8)
		{
			// callCount 5 => 1500 for outer while loop (second iteration) in SpaceEvictionStrategy::doExecute()
			// callCount 6 => 1500 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 1
			// callCount 7 => 1500 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 42
			return 1500L;
		}
		else if (callCount < 9)
		{
			// callCount 8 => 1000 for outer while loop in SpaceEvictionStrategy::doExecute() (to exit the while loop)
			return 1000L;
		}
		else
		{
			// just for safety
			return 0L;
		}
	}
	));
	ON_CALL(*mockBeaconCache, getBeaconIDs())
		.WillByDefault(testing::Return(std::unordered_set<int32_t>({ 1, 42 })));

	// then
	EXPECT_CALL(*mockBeaconCache, getNumBytesInCache())
		.Times(testing::Exactly(8));
	EXPECT_CALL(*mockBeaconCache, evictRecordsByNumber(1, 1))
		.Times(testing::Exactly(2));
	EXPECT_CALL(*mockBeaconCache, evictRecordsByNumber(42, 1))
		.Times(testing::Exactly(2));

	// when
	target.execute();
}

#if 0 // The code currently does not support thread isInterrupted() checking
TEST_F(SpaceEvictionStrategyTest, executeEvictionStopsIfThreadGetsInterruptedBetweenTwoBeacons)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	uint32_t callCount = 0;
	ON_CALL(*mockBeaconCache, getNumBytesInCache())
		.WillByDefault(testing::Invoke(
			[&callCount]() -> int64_t {
		callCount++;
		if (callCount < 2)
		{
			// callCount 1 => 2001 for SpaceEvictionStrategy::shouldRun()
			return 2001L;
		}
		if (callCount < 5)
		{
			// callCount 2 => 2000 for outer while loop in SpaceEvictionStrategy::doExecute()
			// callCount 3 => 2000 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 1
			// callCount 4 => 2000 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 42
			return 2000L;
		}
		else if (callCount < 8)
		{
			// callCount 5 => 1500 for outer while loop (second iteration) in SpaceEvictionStrategy::doExecute()
			// callCount 6 => 1500 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 1
			// callCount 7 => 1500 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 42
			return 1500L;
		}
		else if (callCount < 9)
		{
			// callCount 8 => 1000 for outer while loop in SpaceEvictionStrategy::doExecute() (to exit the while loop)
			return 1000L;
		}
		else
		{
			// just for safety
			return 0L;
		}
	}
	));
	ON_CALL(*mockBeaconCache, getBeaconIDs())
		.WillByDefault(testing::Return(std::unordered_set<int32_t>({ 1, 42 })));

	// then
	EXPECT_CALL(*mockBeaconCache, getNumBytesInCache())
		.Times(testing::Exactly(8));
	EXPECT_CALL(*mockBeaconCache, evictRecordsByNumber(1, 1))
		.Times(testing::Exactly(2));
	EXPECT_CALL(*mockBeaconCache, evictRecordsByNumber(42, 1))
		.Times(testing::Exactly(2));

	// when
	target.execute();
}
#endif

TEST_F(SpaceEvictionStrategyTest, executeEvictionStopsIfNumBytesInCacheFallsBelowLowerBoundBetweenTwoBeacons)
{
	// given
	auto configuration = std::make_shared<BeaconCacheConfiguration>(1000L, 1000L, 2000L);
	SpaceEvictionStrategy target(mockBeaconCache, configuration);

	uint32_t callCount = 0;
	ON_CALL(*mockBeaconCache, getNumBytesInCache())
		.WillByDefault(testing::Invoke(
			[&callCount]() -> int64_t {
		callCount++;
		if (callCount < 2)
		{
			// callCount 1 => 2001 for SpaceEvictionStrategy::shouldRun()
			return 2001L;
		}
		if (callCount < 5)
		{
			// callCount 2 => 2000 for outer while loop in SpaceEvictionStrategy::doExecute()
			// callCount 3 => 2000 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 1
			// callCount 4 => 2000 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 42
			return 2000L;
		}
		else if (callCount < 7)
		{
			// callCount 5 => 1500 for outer while loop (second iteration) in SpaceEvictionStrategy::doExecute()
			// callCount 6 => 1500 for inner while loop in SpaceEvictionStrategy::doExecute() which evicts beaconID 1
			return 1500L;
		}
		else if (callCount < 8)
		{
			// callCount 7 => 1000 for outer while loop in SpaceEvictionStrategy::doExecute() (to exit the while loop)
			return 1000L;
		}
		else
		{
			// just for safety
			return 0L;
		}
	}
	));
	ON_CALL(*mockBeaconCache, getBeaconIDs())
		.WillByDefault(testing::Return(std::unordered_set<int32_t>({ 1, 42 })));

	// then
	EXPECT_CALL(*mockBeaconCache, getNumBytesInCache())
		.Times(testing::Exactly(8));
	EXPECT_CALL(*mockBeaconCache, evictRecordsByNumber(testing::An<int32_t>(), 1))
		.Times(testing::Exactly(3));
	
	// when
	target.execute();
}