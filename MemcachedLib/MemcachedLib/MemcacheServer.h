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
	/**
	 * Server that conatains a data cache for which access to the cache is provided via MBP
	 * Get and Set messages.
	 */
	class MemcacheServer : public MemcacheSocket::ISocketHandler, public Thread::IRunnable
	{
	public:
		/**
		 * Default Ctor
		 */
		MemcacheServer();

		/**
		 * Dtor
		 */
		~MemcacheServer();

		/**
		 * Start the server.
		 *
		 * @param listenPort    TCP port to listen on for incoming connections or 0 to not listen(used for unit tests).
		 *
		 * @return   true - server successfully started, false - could not listen on the provided port.
		 */
		bool Start(unsigned short listenPort);

		/**
		 * Closes all connections and clears the data cache.
		 */
		void Shutdown();

		//
		// IRunnable methods
		//

		/**
		 * Event driven and periodic checks to see if there are any MemcacheSocket instances
		 * that have been disconnected from the remote end and need to be destroyed.
		 *
		 * @param thread    Worker thread that is calling this method
		 */
		virtual void Run(const Thread * thread);

		//
		// ISocketHandler methods
		//

		/**
		 * Accept the incoming connection from a client/server
		 *
		 * @param socket    MemcacheSocket that contains an established connection to a client/server
		 */
		virtual void OnAcceptConn(MemcacheSocket * socket);

		/**
		 * Called by connected MemcacheSocket instances when they receive a message from the remote
		 * client/server.
		 *
		 * @param message    message that was received and parsed from the remote end
		 * @param reply    object to hold the reply message that should be sent back to the remote end on return
		 */
		virtual void OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply);

		/**
		 * Called by MemcacheSocket instances when they have detected that the connection to the remote
		 * client/server has been severed.
		 *
		 * @param sessionID    unique sessionId of the instance to be destroyed.
		 */
		virtual void OnSocketClosed(unsigned int sessionID);

		/**
		 * Clears the data cache of all data and keys.
		 */
		void ClearDataCache();

		/**
		 * Prints all data cache entries to the console.
		 */
		void PrintDataCache();
	protected:
		//
		// typedefs for STL containers
		//
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
		/**
		 * Iterates through the list of closed sockets and destroys them.
		 */
		void ProcessClosedSockets();

		/**
		 * Closes all remaining socket connections to remote clients/servers.
		 */
		void CloseAllSockets();
	};
}
#endif