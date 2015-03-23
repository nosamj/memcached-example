#pragma once
#ifndef MEMCACHE_THREADING_H_
#define MEMCACHE_THREADING_H_

#include <thread>

namespace memcache
{

	/**
	 * Utility class that provides some more controlable threading
	 * functionality. Such as a common/easy way of telling a thread it should terminate
	 */
	class Thread
	{
	public:

		/**
		 * Iterface for allowing any class to do some work on an independent thread.
		 */
		class IRunnable
		{
		public:
			virtual void Run(const Thread * thread) = 0;
		};

	public:
		Thread();
		~Thread();

		/**
		 * Starts the internal thread with the provided interface
		 */
		void Start(IRunnable * runner);

		/**
		 * Stops the thread.
		 *
		 * @param shouldJoin    true - the thread will be joined in this call, false - the thread will not be joined in this call
		 */
		void Stop(bool shouldJoin);

		/**
		 * Typically used by implementations of the IRunnable interface to know
		 * whether or not the should continue executing or return.
		 *
		 * @return   true - run method should exit, false - thread should continue executing
		 */
		bool ShouldTerminate() const
		{
			return _shouldTerminate;
		}

		/**
		 * Returns whether or not the thread is running
		 *
		 * @return   true - thread is running, false - thread is not running
		 */
		bool IsRunning() const
		{
			return _thread.joinable();
		}

		/**
		 * Returns the ID of the thread
		 *
		 * @return   thread id
		 */
		std::thread::id GetID() const { return _thread.get_id(); }

	protected:
		bool _shouldTerminate;
		std::thread _thread;
	};
}
#endif