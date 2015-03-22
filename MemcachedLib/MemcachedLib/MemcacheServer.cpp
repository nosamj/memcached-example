#include <iostream>
#include "MemcacheServer.h"
#include "LockHelper.h"

namespace memcache
{
	MemcacheServer::MemcacheServer() : _threadEvent(false, false)
	{
	}

	MemcacheServer::~MemcacheServer()
	{
		this->Shutdown();
	}

	bool MemcacheServer::Start()
	{
		//
		// start management thread
		//
		_thread.Start(this);

		//
		// start the listen socket so we can accept connections
		//
		if (_listenSock.Listen(30000, this))
		{
			std::cout << "Server started!" << std::endl;
			return true;
		}
		else
		{
			this->Shutdown();
			return false;
		}
	}

	void MemcacheServer::Shutdown()
	{
		//
		// stop the listen socket so that we won't be accepting any incoming connections while
		// we're shutting down.
		//
		_listenSock.Close();

		// close all current connections. This should move them to the socket disposal list
		this->CloseAllSockets();

		// process the closed sockets
		this->ProcessClosedSockets();

		// clear our cache
		this->ClearDataCache();

		// stop the management thread
		_thread.Stop(false);
		_threadEvent.Set();
		_thread.Stop(true);
	}

	void MemcacheServer::Run(const Thread * thread)
	{
		while (!thread->ShouldTerminate())
		{
			_threadEvent.TimedWait(100);

			if (thread->ShouldTerminate())
			{
				break;
			}
			else
			{
				this->ProcessClosedSockets();
			}
		}
	}

	void MemcacheServer::OnAcceptConn(MemcacheSocket * socket)
	{
		LockHelper myLock(_sockMapMutex);

		_socketMap[socket->GetSessionID()].reset(socket);
	}

	void MemcacheServer::OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply)
	{

	}

	void MemcacheServer::OnSocketClosed(unsigned int sessionID)
	{
		LockHelper myLock(_sockMapMutex);

		SocketMap_t::iterator it = _socketMap.find(sessionID);
		if (it != _socketMap.end())
		{
			_sockListMutex.Lock();
			_socketDisposal.push_back(it->second);
			_sockListMutex.Unlock();
			_threadEvent.Set();

			_socketMap.erase(it);
		}
	}

	void MemcacheServer::ProcessClosedSockets()
	{
		LockHelper myLock(_sockListMutex);

		SocketList_t::iterator it, endIt = _socketDisposal.end();
		for (it = _socketDisposal.begin();
			it != endIt;
			++it)
		{
			it->reset();
		}

		_socketDisposal.clear();
	}

	void MemcacheServer::CloseAllSockets()
	{
		LockHelper myLock(_sockMapMutex);
		for (SocketMap_t::iterator it = _socketMap.begin();
			it != _socketMap.end();
			/*iterator moved in loop*/)
		{
			if (it->second)
			{
				it->second->Close();
				_sockListMutex.Lock();
				_socketDisposal.push_back(it->second);
				_sockListMutex.Unlock();
			}
			it = _socketMap.erase(it);
		}
	}

	void MemcacheServer::ClearDataCache()
	{
		LockHelper myLock(_dataMapLock.WriteMutex());

		_dataMap.clear();
	}
}