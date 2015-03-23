#pragma once
#ifndef MEMCACHE_MEMCACHECLIENT_H_
#define MEMCACHE_MEMCACHECLIENT_H_

#include "MemcacheMessages.h"
#include "MemcacheSocket.h"

namespace memcache
{
	/**
	 * Class for connecting to a Memcache server and synchronously
	 * issuing Set and Get requests.
	 */
	class MemcacheClient : public MemcacheSocket::ISocketHandler
	{
	public:
		/**
		 * Default Ctor
		 */
		MemcacheClient();

		/**
		 * Will disconnect the socket if still connected.
		 */
		~MemcacheClient();

		/**
		 * Connect to the memcache server.
		 *
		 * @param address    IP address of the server
		 * @param port    port that the server is listening on
		 *
		 * @return   true - successfully connected, false - failed to connect
		 */
		bool Connect(const std::string & address, unsigned short port);

		/**
		 * Disconnects from the server
		 */
		void Disconnect();

		//
		// ISocketHandler methods
		//

		/**
		 * Not applicable since the internal socket is not a listening socket.
		 */
		virtual void OnAcceptConn(MemcacheSocket * socket);

		/**
		 * Called when the socket receives a valid message from the server.
		 *
		 * @param message    received message
		 * @param reply    stores the reply message to send back to the server.
		 */
		virtual void OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply);

		/**
		 * Not applicable since the select thread of the socket used by this class won't
		 * be running.
		 */
		virtual void OnSocketClosed(unsigned int sessionID);


		/**
		 * Sends a Set message to the connected server with the provided key, flags and value.
		 *
		 * @param key    key to set the value of
		 * @param flags    flags to send in the set
		 * @param value    value data to set
		 *
		 * @return   true - data has been set, false - set failed or not connected to server.
		 */
		bool Set(const std::string & key, unsigned int flags, const DataBuffer & value);

		/**
		 * Sends a Get message to the connected server with the provided key.
		 *
		 * @param key    key to get the value of
		 * @param outValue    buffer to hold the value if found
		 * @param outFlags    flags returned if entry is found
		 *
		 * @return true - entry found on server and data placed in outValue, false - entry matching key was not found or not connected to server.
		 */
		bool Get(const std::string & key, DataBuffer & outValue, unsigned int & flags);

	protected:
		std::unique_ptr<MemcacheSocket> _socket;///< socket used to send/receive messages with the server
		std::unique_ptr<BaseMessage> _readMsg;///< stores the last message received from the server
	};
}
#endif