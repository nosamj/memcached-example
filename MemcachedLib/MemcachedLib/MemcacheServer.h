#pragma once
#ifndef MEMCACHE_MEMCACHESERVER_H_
#define MEMCACHE_MEMCACHESERVER_H_

#include <string>
#include <list>
#include <map>
#include "ReadWriteLock.h"
#include "DataBuffer.h"
#include "MemcacheMessages.h"
#include "MemcacheSocket.h"

namespace memcache
{
	class MemcacheServer : public MemcacheSocket::ISocketHandler, public Thread::IRunnable
	{
	public:
		MemcacheServer();
		~MemcacheServer();

		bool Start(unsigned short listenPort = 30000);
		void Shutdown();

		//
		// IRunnable methods
		//
		virtual void Run(const Thread * thread);

		//
		// ISocketHandler methods
		//
		virtual void OnAcceptConn(MemcacheSocket * socket);
		virtual void OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply);
		virtual void OnSocketClosed(unsigned int sessionID);

	protected:
		typedef std::map<unsigned int, std::shared_ptr<MemcacheSocket> > SocketMap_t;
		typedef std::list<std::shared_ptr<MemcacheSocket> > SocketList_t;
		typedef struct DataEntry
		{
			unsigned int Flags;
			std::shared_ptr<DataBuffer> DataBuffer;
		}DataEntry_t;
		typedef std::map<std::string, DataEntry_t> DataMap_t;

		DataMap_t _dataMap;///< map that stores all of the data entries.
		ReadWriteLock _dataMapLock;///< used to protect the cache
		SocketMap_t _socketMap;///< map for storing all of the client sockets
		Mutex _sockMapMutex;///< controls access to the socket map
		SocketList_t _socketDisposal;///< holds disconnected sockets until they can be destroyed.
		Mutex _sockListMutex;///< controls access to the dead sockets list
		MemcacheSocket _listenSock;///< socket that will be used to listen for incoming connections
		Thread _thread;///< thread that will be used to remove disconnected sockets
		Event _threadEvent;///< event that will be used to signal the management thread of the server to remove closed sockets

	protected:
		void ProcessClosedSockets();
		void CloseAllSockets();
		void ClearDataCache();
	};
}
#endif