#pragma once
#ifndef MEMCACHE_THREADING_H_
#define MEMCACHE_THREADING_H_

#include <thread>

namespace memcache
{
	class Thread
	{
	public:
		class IRunnable
		{
		public:
			virtual void Run(const Thread * thread) = 0;
		};

	public:
		Thread();
		~Thread();
		void Start(IRunnable * runner);
		void Stop(bool shouldJoin);
		bool ShouldTerminate() const
		{
			return _shouldTerminate;
		}

		bool IsRunning() const
		{
			return _thread.joinable();
		}

	protected:
		bool _shouldTerminate;
		std::thread _thread;
	};
}
#endif