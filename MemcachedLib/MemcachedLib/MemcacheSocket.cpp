#include <atomic>
#include <iostream>
#include "MemcacheSocket.h"

namespace memcache
{
	unsigned int GetNextSessionID()
	{
		static std::atomic_uint nextSessionID;

		return ++nextSessionID;
	}

	MemcacheSocket::MemcacheSocket() :
		_socket(INVALID_SOCKET),
		_sessionID(GetNextSessionID()),
		_handler(nullptr),
		_isListening(false)
	{
	}

	MemcacheSocket::MemcacheSocket(MemcacheSocket::ISocketHandler * handler) :
		_socket(INVALID_SOCKET),
		_sessionID(GetNextSessionID()),
		_handler(handler),
		_isListening(false)
	{
	}
	
	bool MemcacheSocket::Listen(unsigned short port, MemcacheSocket::ISocketHandler * handler)
	{
		this->Close();

		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			return false;
		}

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = INADDR_ANY;
		addr.sin_port = htons(port);
		int res = bind(_socket, (const sockaddr *)&addr, (int)sizeof(addr));
		if (res == SOCKET_ERROR)
		{
			std::cout << "Failed to bind on port " << port << std::endl;
			this->Close();
			return false;
		}
		else
		{
			std::cout << "Successfully bound on port " << port << std::endl;
		}

		res = listen(_socket, SOMAXCONN);
		if (res == SOCKET_ERROR)
		{
			std::cout << "listen call failed" << std::endl;
			this->Close();
			return false;
		}
		
		//
		// make sure to assign our handler BEFORE we start our thread so we don't
		// miss any incoming connections
		//
		_handler = handler;

		//
		// now that the socket is listening let's start up the select thread
		// so that we can accept the incoming connections
		//
		_selectThread.Start(this);

		return true;
	}

	void MemcacheSocket::Close()
	{
		//
		// stop the select thread without joining
		// so that we don't have to wait for the current select call
		// to timeout. Instead, we'll close our socket before attempting
		// to join the thread.
		//
		_selectThread.Stop(false);

		if (INVALID_SOCKET != _socket)
		{
			closesocket(_socket);
			_socket = INVALID_SOCKET;
		}

		_selectThread.Stop(true);

		if (nullptr != _handler)
		{
			_handler->OnSocketClosed(_sessionID);
		}
	}
}