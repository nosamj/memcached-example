#include "Mutex.h"

namespace memcache
{
	void Mutex::Lock()
	{
		_mutex.lock();
	}

	void Mutex::Unlock()
	{
		_mutex.unlock();
	}
}