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

#include "BeaconCache.h"

#include <mutex> 

using namespace caching;
 
BeaconCache::BeaconCache()
	: observers()
	, mGlobalCacheLock()
	, mBeacons()
	, mCacheSizeInBytes(0)
{

}

void BeaconCache::addObserver(IObserver* observer)
{
	observers.push_back(observer);
}

void BeaconCache::addEventData(int32_t beaconID, int64_t timestamp, const core::UTF8String& data)
{
	// get a reference to the cache entry
	auto entry = getCachedEntryOrInsert(beaconID);

	BeaconCacheRecord record(timestamp, data);
	
	std::unique_lock<std::mutex> lock(entry->getLock());
	entry->addEventData(record);
	lock.unlock();

	// update cache stats
	mCacheSizeInBytes += record.getDataSizeInBytes();

	// notify observers
	onDataAdded();
}

void BeaconCache::addActionData(int32_t beaconID, int64_t timestamp, const core::UTF8String& data)
{
	// get a reference to the cache entry
	auto entry = getCachedEntryOrInsert(beaconID);

	BeaconCacheRecord record(timestamp, data);
	std::unique_lock<std::mutex> lock(entry->getLock());
	entry->addActionData(record);
	lock.unlock();

	// update cache stats
	mCacheSizeInBytes += record.getDataSizeInBytes();

	// notify observers
	onDataAdded();
}

void BeaconCache::deleteCacheEntry(int32_t beaconID)
{
	mGlobalCacheLock.WriteLock();

	auto it = mBeacons.find(beaconID);
	if (it != mBeacons.end())
	{
		mCacheSizeInBytes -= it->second->getTotalNumberOfBytes();
		mBeacons.erase(it);
	}
	
	mGlobalCacheLock.WriteUnlock();
}

const core::UTF8String BeaconCache::getNextBeaconChunk(int32_t beaconID, const core::UTF8String& chunkPrefix, int32_t maxSize, const char* delimiter)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// a cache entry for the given beaconID does not exist
		return core::UTF8String();
	}

	if (entry->needsDataCopyBeforeChunking())
	{
		// both entries are null, prepare data for sending
		int64_t numBytes = 0;
		std::unique_lock<std::mutex> lock(entry->getLock());
		numBytes = entry->getTotalNumberOfBytes();
		entry->copyDataForChunking();
		lock.unlock();

		// assumption: sending will work fine, and everything we copied will be removed quite soon
		mCacheSizeInBytes -= numBytes;
	}

	// data for chunking is available
	return entry->getChunk(chunkPrefix, maxSize, delimiter);
}

void BeaconCache::removeChunkedData(int32_t beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// a cache entry for the given beaconID does not exist
		return;
	}

	entry->removeDataMarkedForSending();
}

void BeaconCache::resetChunkedData(int32_t beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// a cache entry for the given beaconID does not exist
		return;
	}

	int64_t numBytes = 0;
	std::unique_lock<std::mutex> lock(entry->getLock());
	int64_t oldSize = entry->getTotalNumberOfBytes();
	entry->resetDataMarkedForSending();
	int64_t newSize = entry->getTotalNumberOfBytes();
	numBytes = newSize - oldSize;
	lock.unlock();

	mCacheSizeInBytes += numBytes;

	// notify observers
	onDataAdded();
}

std::shared_ptr<BeaconCacheEntry> BeaconCache::getCachedEntryOrInsert(int beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// does not exist, and needs to be inserted
		mGlobalCacheLock.WriteLock();

		// double check since this could have been added in the mean time
		auto it = mBeacons.find(beaconID);
		if (it == mBeacons.end())
		{
			entry = std::make_shared<BeaconCacheEntry>();
			mBeacons.insert(std::make_pair(beaconID, entry));
		}
		else
		{
			entry = it->second;
		}

		mGlobalCacheLock.WriteUnlock();
	}

	return entry;
}

const std::vector<core::UTF8String> BeaconCache::getEvents(int32_t beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// entry not found
		return {};
	}

	std::vector<core::UTF8String> events;
	std::unique_lock<std::mutex> lock(entry->getLock());
	events = extractData(entry->getEventData());
	lock.unlock();
	return events;
}

const std::list<BeaconCacheRecord> BeaconCache::getEventsBeingSent(int32_t beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// entry not found
		return {};
	}

	return entry->getEventDataBeingSent();
}

const std::vector<core::UTF8String> BeaconCache::getActions(int32_t beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// entry not found
		return {};
	}

	std::vector<core::UTF8String> actions;
	std::unique_lock<std::mutex> lock(entry->getLock());
	actions = extractData(entry->getActionData());
	lock.unlock();
	return actions;
}

const std::list<BeaconCacheRecord> BeaconCache::getActionsBeingSent(int32_t beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// entry not found
		return {};
	}

	return entry->getActionDataBeingSent();
}

std::vector<core::UTF8String> BeaconCache::extractData(const std::list<BeaconCacheRecord>& eventData)
{
	std::vector<core::UTF8String> result;
	for (auto const& value : eventData)
	{
		result.push_back(value.getData());
	}
	return result;
}

std::shared_ptr<BeaconCacheEntry> BeaconCache::getCachedEntry(int32_t beaconID)
{
	std::shared_ptr<BeaconCacheEntry> entry = nullptr;

	// acquire read lock and get the entry
	mGlobalCacheLock.ReadLock();
	auto it = mBeacons.find(beaconID);
	if (it != mBeacons.end())
	{
		entry = it->second;
	}
	mGlobalCacheLock.ReadUnlock();

	return entry;
}

const std::unordered_set<int32_t> BeaconCache::getBeaconIDs()
{
	std::unordered_set<int32_t> result;
	
	mGlobalCacheLock.ReadLock();
	for (auto const& beacon : mBeacons)
	{
		result.insert(beacon.first);
	}

	mGlobalCacheLock.ReadUnlock();

	return result;
}

uint32_t BeaconCache::evictRecordsByAge(int32_t beaconID, int64_t minTimestamp)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// already removed
		return 0;
	}

	std::unique_lock<std::mutex> lock(entry->getLock());
	uint32_t numRecordsRemoved = entry->removeRecordsOlderThan(minTimestamp);
	lock.unlock();

	return numRecordsRemoved;
}

uint32_t BeaconCache::evictRecordsByNumber(int32_t beaconID, uint32_t numRecords)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// already removed
		return 0;
	}

	std::unique_lock<std::mutex> lock(entry->getLock());
	uint32_t numRecordsRemoved = entry->removeOldestRecords(numRecords);
	lock.unlock();

	return numRecordsRemoved;
}

int64_t const BeaconCache::getNumBytesInCache() const
{
	return mCacheSizeInBytes;
}

void BeaconCache::onDataAdded()
{
	for (std::vector<IObserver*>::const_iterator iter = observers.begin(); iter != observers.end(); ++iter)
	{
		if ((*iter) != nullptr)
		{
			(*iter)->update();
		}
	}
}

bool BeaconCache::isEmpty(int32_t beaconID)
{
	auto entry = getCachedEntry(beaconID);
	if (entry == nullptr)
	{
		// already removed
		return true;
	}

	std::unique_lock<std::mutex> lock(entry->getLock());
	bool isEmpty = entry->getTotalNumberOfBytes() == 0;
	lock.unlock();
	
	return isEmpty;
}