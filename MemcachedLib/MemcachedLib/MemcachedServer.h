#pragma once
#ifndef MEMCACHED_MEMCACHEDSERVER_H_
#define MEMCACHED_MEMCACHEDSERVER_H_

#include <string>
#include <map>
#include "ReadWriteLock.h"
#include "DataBuffer.h"

namespace memcached
{
	class MemcachedServer
	{
	public:
		MemcachedServer();
		~MemcachedServer();

	protected:
		typedef std::map<std::string, DataBuffer *> DataMap_t;
		DataMap_t _dataMap;
		ReadWriteLock _mapLock;///< used to protect the cache
	};
}
#endif