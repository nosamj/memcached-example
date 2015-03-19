#include "Mutex.h"

namespace memcached
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