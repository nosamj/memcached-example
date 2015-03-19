#include "ReadWriteLock.h"
#include "LockHelper.h"

namespace memcached
{
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