#pragma once
#ifndef _MEMCACHE_MUTEX_H_
#define _MEMCACHE_MUTEX_H_

#include <mutex>

namespace memcache
{

	/**
	 * Interface for a simple mutex
	 */
	class IMutex
	{
	public:
		/**
		 * Lock the mutex
		 */
		virtual void Lock() = 0;

		/**
		 * Unlock the mutex
		 */
		virtual void Unlock() = 0;
	};

	/**
	 * Recursive mutex
	 */
	class Mutex : public IMutex
	{
	public:
		Mutex() {}
		virtual ~Mutex() {}
		virtual void Lock();
		virtual void Unlock();

	protected:
		std::recursive_mutex _mutex;
	};
}
#endif