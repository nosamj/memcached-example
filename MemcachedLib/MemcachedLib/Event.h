#pragma once
#ifndef _MEMCACHE_EVENT_H_
#define _MEMCACHE_EVENT_H_

#include <WinSock2.h>

namespace memcache
{

	/**
	 * Class used for signaling threads to wake up when
	 * some event/action has taken place.
	 */
	class Event
	{
	public:

		/**
		 * Ctor:
		 *
		 * @param manualReset    true - the event has to be reset manually. All threads waiting on an event will be awoken.
		 * false - the event will be reset automatically by the first waiting thread that is awoken by this event being set.
		 */
		Event(bool manualReset, bool initialState);

		/**
		 * Dtor
		 */
		~Event();

		/**
		 * Sets the event to high. AKA signals the event
		 */
		void Set();

		/**
		 * Sets the event to low.
		 */
		void Reset();

		/**
		 * Waits indefintiely for the event to be set
		 */
		void Wait();

		/**
		 * Waits the provided amount of time for the event to be set
		 *
		 * @param waitMS    amount of time to wait in milliseconds
		 */
		void TimedWait(unsigned int waitMS);
	protected:
		HANDLE _handle;///< native handle to the system event
	};
}
#endif