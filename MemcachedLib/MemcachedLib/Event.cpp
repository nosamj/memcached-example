#include "Event.h"

namespace memcache
{
	Event::Event(bool manualReset, bool initialState) : _handle(NULL)
	{
		_handle = CreateEvent(NULL, manualReset, initialState, NULL);
	}

	Event::~Event()
	{
		if (NULL != _handle)
		{
			CloseHandle(_handle);
			_handle = NULL;
		}
	}

	void Event::Reset()
	{
		ResetEvent(_handle);
	}

	void Event::Set()
	{
		SetEvent(_handle);
	}

	void Event::Wait()
	{
		WaitForSingleObject(_handle, INFINITE);
	}
}