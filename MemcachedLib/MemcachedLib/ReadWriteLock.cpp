#include "ReadWriteLock.h"
#include "LockHelper.h"

namespace memcache
{
	/**
	 * Helper class for using the LockHelper class to lock/unlock as a reader
	 * of the ReadWriteLock class.
	 */
	class ReadLock : public IMutex
	{
	public:
		ReadLock(ReadWriteLock & lock) : _lock(lock) {}
		virtual void Lock() { _lock.LockRead(); }
		virtual void Unlock() { _lock.UnlockRead(); }

	protected:
		ReadWriteLock & _lock;
	};

	/**
	 * Helper class for using the LockHelper class to lock/unlock as a writer
	 * of the ReadWriteLock class.
	 */
	class WriteLock : public IMutex
	{
	public:
		WriteLock(ReadWriteLock & lock) : _lock(lock) {}
		virtual void Lock() { _lock.LockWrite(); }
		virtual void Unlock() { _lock.UnlockWrite(); }

	protected:
		ReadWriteLock & _lock;
	};

	ReadWriteLock::ReadWriteLock() 
		: _readEvent(true, true),
		_writeEvent(true, true),
		_writeCount(0)
	{
		_readLock = new ReadLock(*this);
		_writeLock = new WriteLock(*this);
	}

	ReadWriteLock::~ReadWriteLock()
	{
		delete _readLock;
		_readLock = nullptr;

		delete _writeLock;
		_writeLock = nullptr;
	}

	void ReadWriteLock::LockRead()
	{
		// make sure there is no thread with the write lock
		_writeEvent.Wait();

		++_readCount;
	}

	void ReadWriteLock::UnlockRead()
	{
		if (--_readCount == 0)
		{
			_readEvent.Set();
		}
	}

	void ReadWriteLock::LockWrite()
	{
		// lock to prevent other write threads from also getting write access.
		_writeMutex.Lock();

		// reset the write event to stop new readers
		_writeEvent.Reset();

		if (_writeCount++ == 0)
		{
			// wait for all readers to complete
			_readEvent.Wait();
		}
	}

	void ReadWriteLock::UnlockWrite()
	{
		if (--_writeCount == 0)
		{
			// signal that the final write accessor is finished
			_writeEvent.Set();
		}

		_writeMutex.Unlock();
	}
}