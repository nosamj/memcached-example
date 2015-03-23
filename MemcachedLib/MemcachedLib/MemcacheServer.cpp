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

	bool MemcacheServer::Start(unsigned short listenPort)
	{
		//
		// start management thread
		//
		_thread.Start(this);

		if (listenPort > 0)
		{
			//
			// start the listen socket so we can accept connections
			//
			if (_listenSock.Listen(listenPort, this))
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
		return true;
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
		MessageHeader header = message->GetHeader();
		if (header.Magic == kMagicReq)
		{
			switch (header.Opcode)
			{
			case kOpGet:
			{
				LockHelper myLock(_dataMapLock.ReadMutex());
				GetRequest * theReq = static_cast<GetRequest*>(message);
				GetResponse * theRes = new GetResponse();
				reply.reset(theRes);
				reply->SetKey(theReq->GetKey());
				DataMap_t::iterator it = _dataMap.find(theReq->GetKey());
				if (it != _dataMap.end())
				{
					if (it->second.DataBuffer)
					{
						theRes->SetFlags(it->second.Flags);
						theRes->SetValue(*it->second.DataBuffer.get());
						reply->SetStatus(kResNoError);
					}
					else
					{
						reply->SetStatus(kResNotStored);
					}
				}
				else
				{
					// entry was not found so return a response with a Not Found status
					reply->SetStatus(kResKeyNotFound);
				}
			}
				break;
			case kOpSet:
			{
				SetRequest * theReq = static_cast<SetRequest *>(message);
				SetResponse * theRes = new SetResponse();
				//
				// create a new map entry and set the flags
				// and value from the incoming request message
				//
				DataMap_t::mapped_type entry;
				entry.Flags = theReq->GetFlags();
				entry.DataBuffer.reset(new DataBuffer());
				*entry.DataBuffer.get() = theReq->GetValue();

				//
				// now that the data has been set let's insert this entry into our map
				//
				_dataMapLock.LockWrite();
				_dataMap[theReq->GetKey()] = entry;
				_dataMapLock.UnlockWrite();

				// build our reply
				reply.reset(theRes);
				reply->SetStatus(kResNoError);
			}
				break;
			default:
				break;
			}
		}
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

	void MemcacheServer::PrintDataCache()
	{
		LockHelper myLock(_dataMapLock.ReadMutex());

		std::cout << "DataCache (" << _dataMap.size() << " entries)" << std::endl;
		DataMap_t::iterator it, endIt = _dataMap.end();
		for (it = _dataMap.begin();
			it != endIt;
			++it)
		{
			std::cout << "Key:" << it->first << " Flags:" << it->second.Flags;
			if (it->second.DataBuffer)
			{
				std::cout << " DataSize:" << it->second.DataBuffer->GetBytesWritten();
			}
			std::cout << std::endl;
		}
	}
}