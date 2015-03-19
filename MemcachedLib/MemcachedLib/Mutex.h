#pragma once
#ifndef _MEMCACHED_MUTEX_H_
#define _MEMCACHED_MUTEX_H_

#include <mutex>

namespace memcached
{
	class IMutex
	{
	public:
		virtual void Lock() = 0;
		virtual void Unlock() = 0;
	};

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