#pragma once
#ifndef MEMCACHE_MEMCACHESERVER_H_
#define MEMCACHE_MEMCACHESERVER_H_

#include <string>
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

		bool Start();
		void Shutdown();

		//
		// IRunnable methods
		//
		virtual void Run(const Thread * thread);

		//
		// ISocketHandler methods
		//
		virtual void OnAcceptConn(MemcacheSocket * socket);
		virtual void OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & response);
		virtual void OnSocketClosed(unsigned int sessionID);

	protected:
		typedef std::map<unsigned int, std::unique_ptr<MemcacheSocket> > SocketMap_t;
		typedef std::map<std::string, std::unique_ptr<DataBuffer> > DataMap_t;
		DataMap_t _dataMap;
		ReadWriteLock _mapLock;///< used to protect the cache
		MemcacheSocket _listenSock;
		Thread _thread;
	};
}
#endif