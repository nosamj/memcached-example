#include "Threading.h"

namespace memcache
{
	void ThreadEntry(const Thread * thread, Thread::IRunnable * runner)
	{
		if (runner != nullptr)
		{
			runner->Run(thread);
		}
	}

	Thread::Thread() : _shouldTerminate(false)
	{
	}

	Thread::~Thread()
	{
		this->Stop(true);
	}

	void Thread::Start(Thread::IRunnable * runner)
	{
		this->Stop(true);

		_shouldTerminate = false;

		_thread = std::thread(ThreadEntry, this, runner);
	}

	void Thread::Stop(bool shouldJoin)
	{
		_shouldTerminate = true;

		if (shouldJoin && _thread.joinable())
		{
			_thread.join();
		}
	}
}