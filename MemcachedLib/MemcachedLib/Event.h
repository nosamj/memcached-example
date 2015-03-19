#pragma once
#ifndef _MEMCACHE_EVENT_H_
#define _MEMCACHE_EVENT_H_

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
	};
}
#endif