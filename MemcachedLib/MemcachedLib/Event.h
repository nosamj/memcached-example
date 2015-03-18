#pragma once
#ifndef _MEMCACHED_EVENT_H_
#define _MEMCACHED_EVENT_H_

namespace memcached
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