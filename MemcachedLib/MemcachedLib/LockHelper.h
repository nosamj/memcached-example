#pragma once
#ifndef _MEMCACHE_LOCKHELPER_H_
#define _MEMCACHE_LOCKHELPER_H_

#include "Mutex.h"

namespace memcache
{

	/**
	 * utility class that will lock a mutex for the lifetime/scope of a LockHelper instance.
	 */
	class LockHelper
	{
	public:
		/**
		 * Ctor
		 *
		 * @param mutex    mutex to lock
		 */
		LockHelper(IMutex & mutex) : _mutex(mutex)
		{
			_mutex.Lock();
		}

		/**
		 * Ctor
		 *
		 * @param mutex    mutex to lock
		 */
		LockHelper(IMutex * mutex) : _mutex(*mutex)
		{
			_mutex.Lock();
		}

		/**
		 * Dtor that unlocks the provided mutex
		 */
		~LockHelper()
		{
			_mutex.Unlock();
		}
	protected:
		IMutex & _mutex;///< reference to the abstract mutex to lock/unlock
	};
}
#endif