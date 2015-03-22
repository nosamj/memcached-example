#pragma once
#ifndef _MEMCACHE_EVENT_H_
#define _MEMCACHE_EVENT_H_

#include <WinSock2.h>

namespace memcache
{
	class Event
	{
	public:
		Event(bool manualReset, bool initialState);
		~Event();
		void Set();
		void Reset();
		void Wait();
		void TimedWait(unsigned int waitMS);
	protected:
		HANDLE _handle;
	};
}
#endif