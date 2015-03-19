#pragma once
#ifndef _MEMCACHED_READWRITELOCK_H_
#define _MEMCACHED_READWRITELOCK_H_

#include <atomic>
#include "Event.h"
#include "Mutex.h"

namespace memcache
{
	/**
	 * Forward declaration for the ReadLock and WriteLock classes
	 */
	class ReadWriteLock;

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

	/**
	 * Class that allows for multiple read threads to access some
	 * resource simultaneously while only one writer can write to
	 * said resource at a time. Essentially, readers don't block
	 * other readers but a writer blocks all readers.
	 */
	class ReadWriteLock
	{
	public:
		ReadWriteLock();
		~ReadWriteLock();

		/**
		 * Gets read access. Blocks while writer completes.
		 */
		void LockRead();

		/**
		 * Releases read access.
		 */
		void UnlockRead();

		/**
		 * Gets write access. Waits for all readers to complete.
		 */
		void LockWrite();

		/**
		 * Releases write access.
		 */
		void UnlockWrite();

		/**
		 * Returns a helper mutex for gaining/releasing read access
		 *
		 * @return   Mutex that will lock/unlock read
		 */
		IMutex * ReadMutex();

		/**
		 * Returns a helper mutex for gaining/releasing write access
		 *
		 * @return   Mutex that will lock/unlock write
		 */
		IMutex * WriteMutex();

	protected:
		std::atomic_uint _readCount; ///< tracks the number of readers.
		unsigned int _writeCount; ///< tracks when the final write method done
		Mutex _writeMutex; ///< mutex for allowing only one write thread
		Event _readEvent; ///< signaled when all readers are done
		Event _writeEvent; ///< signaled what write is compelte and unsignaled to prevent new readers.
		ReadLock * _readLock;
		WriteLock * _writeLock;
	};
}
#endif