#pragma once
#ifndef _MEMCACHED_LOCKHELPER_H_
#define _MEMCACHED_LOCKHELPER_H_

#include "Mutex.h"

namespace memcache
{
	class LockHelper
	{
	public:
		LockHelper(IMutex & mutex) : _mutex(mutex)
		{
			_mutex.Lock();
		}

		LockHelper(IMutex * mutex) : _mutex(*mutex)
		{
			_mutex.Lock();
		}

		~LockHelper()
		{
			_mutex.Unlock();
		}
	protected:
		IMutex & _mutex;
	};
}
#endif