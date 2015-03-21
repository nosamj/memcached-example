#include <iostream>
#include "Threading.h"
#include "ReadWriteLock.h"
#include "catch.hpp"

using namespace memcache;

TEST_CASE("Thread start and is running")
{
	class SleepyRunner : public Thread::IRunnable
	{
	public:
		virtual void Run(const Thread * thread)
		{
			while (!thread->ShouldTerminate())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
	};
	SleepyRunner runner;
	Thread thread;

	REQUIRE_FALSE(thread.IsRunning());

	thread.Start(&runner);

	REQUIRE(thread.IsRunning());
	REQUIRE(thread.GetID() != std::this_thread::get_id());

	thread.Stop(true);

	REQUIRE_FALSE(thread.IsRunning());
}

TEST_CASE("ReadWriteLock")
{
	class Reader : public Thread::IRunnable
	{
	public:
		Reader(ReadWriteLock & lock) : _lock(lock), _isReading(false) {}

		bool IsReading() const { return _isReading; }
		virtual void Run(const Thread * thread)
		{
			while (!thread->ShouldTerminate())
			{
				_lock.LockRead();
				_isReading = true;

				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				_isReading = false;
				_lock.UnlockRead();
			}
		}

	protected:
		ReadWriteLock & _lock;
		bool _isReading;
	};

	class Writer : public Thread::IRunnable
	{
	public:
		Writer(ReadWriteLock & lock) : _lock(lock) , _event(true, false) {}
		void StopWriting() { _event.Set(); }
		virtual void Run(const Thread * thread)
		{
			while (!thread->ShouldTerminate())
			{
				_lock.LockWrite();
				_event.Set();
				_event.Reset();
				_event.Wait();
				_lock.UnlockWrite();
			}
		}

		Event _event;

	protected:
		ReadWriteLock & _lock;
	};
	ReadWriteLock lock;
	Reader reader1(lock), reader2(lock), reader3(lock);
	Writer writer(lock);
	Thread t1, t2, t3, t4;

	//
	// start the 3 readers and confirm that they're all reading.
	//
	t1.Start(&reader1);
	t2.Start(&reader2);
	t3.Start(&reader3);
	REQUIRE(reader1.IsReading());
	REQUIRE(reader2.IsReading());
	REQUIRE(reader3.IsReading());


	//
	// start the writer and wait for him to signal
	// that he has obtained the write lock
	//
	//std::cout << "Starting writer...";
	t4.Start(&writer);
	//std::cout << "started." << std::endl << "Waiting for write event to be signaled...";
	writer._event.Wait();
	//std::cout << "WriteLock obtained." << std::endl;

	//
	// now that the writer has the write lock let's confirm that
	// none of the readers are reading.
	//
	REQUIRE_FALSE(reader1.IsReading());
	REQUIRE_FALSE(reader2.IsReading());
	REQUIRE_FALSE(reader3.IsReading());

	//
	// stop writing and now confirm that the readers are once again reading.
	//
	t4.Stop(false);
	writer.StopWriting();

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	REQUIRE(reader1.IsReading());
	REQUIRE(reader2.IsReading());
	REQUIRE(reader3.IsReading());

	//
	// stop all threads
	//
	t1.Stop(true);
	t2.Stop(true);
	t3.Stop(true);
	t4.Stop(true);
}