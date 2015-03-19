#pragma once
#ifndef MEMCACHE_MEMCACHESERVER_H_
#define MEMCACHE_MEMCACHESERVER_H_

#include <string>
#include <map>
#include "ReadWriteLock.h"
#include "DataBuffer.h"
#include "MemcacheMessages.h"

namespace memcache
{
	class MemcacheServer : public IMessageHandler
	{
	public:
		MemcacheServer();
		~MemcacheServer();

	protected:
		typedef std::map<std::string, std::unique_ptr<DataBuffer> > DataMap_t;
		DataMap_t _dataMap;
		ReadWriteLock _mapLock;///< used to protect the cache
	};
}
#endif