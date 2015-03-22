#pragma once
#ifndef MEMCACHE_MEMCACHECLIENT_H_
#define MEMCACHE_MEMCACHECLIENT_H_

#include "MemcacheMessages.h"
#include "MemcacheSocket.h"

namespace memcache
{
	class MemcacheClient : public MemcacheSocket::ISocketHandler
	{
	public:
		MemcacheClient();
		~MemcacheClient();

		bool Connect(const std::string & address, unsigned short port);
		void Disconnect();

		//
		// ISocketHandler methods
		//
		virtual void OnAcceptConn(MemcacheSocket * socket);
		virtual void OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply);
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
		std::unique_ptr<MemcacheSocket> _socket;
		std::unique_ptr<BaseMessage> _readMsg;
	};
}
#endif