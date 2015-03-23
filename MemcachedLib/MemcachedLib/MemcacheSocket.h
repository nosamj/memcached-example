#include <WinSock2.h>
#include <string>
#include "MemcacheDefs.h"
#include "MemcacheMessages.h"
#include "Threading.h"

namespace memcache
{

	/**
	 * Class that provides TCP socket communication abilities specically for
	 * sending and receiving of MBP messages.
	 */
	class MemcacheSocket : Thread::IRunnable
	{
	public:
		/**
		 * Interface for sending events and data up to the owner of the socket
		 */
		class ISocketHandler
		{
		public:
			virtual void OnAcceptConn(MemcacheSocket * socket) = 0;
			virtual void OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply) = 0;
			virtual void OnSocketClosed(unsigned int sessionID) = 0;
		};

	public:
		/**
		 * Default Ctor
		 */
		MemcacheSocket();

		/**
		 * Ctor that takes in a handler. Typically used with clients
		 *
		 * @param handler    handler to use for events
		 */
		MemcacheSocket(ISocketHandler * handler);

		/**
		 * Ctor that takes in an existing socket connection and handler
		 *
		 * @param socket    existing connected socket handle
		 * @param handler    handler to use for events
		 */
		MemcacheSocket(SOCKET socket, ISocketHandler * handler);

		/**
		 * Dtor. Closes the socket if it is still open.
		 */
		~MemcacheSocket();

		/**
		 * Starts listening on the provided port on all interfaces.
		 *
		 * @param port    TCP port to listen on
		 * @param handler    handler to use for accepting connections and pass to all accepted sockets
		 *
		 * @return   true - successfully listening on port, false - failed to listen on the provided port
		 */
		bool Listen(unsigned short port, ISocketHandler * handler);

		/**
		 * Connect to the remote address on the specified port
		 *
		 * @param address    remote IP address
		 * @param port    remote port
		 * @param useAsyncRead    true - a thread will be created for reading messages from the socket, false - no thread will be created. Data must be read manually with ReadMessage
		 *
		 * @return   true - successfully connected, false - failed to connect
		 */
		bool Connect(const std::string & address, unsigned short port, bool useAsyncRead);

		/**
		 * Closes the socket and stops the read thread if runnin
		 */
		void Close();

		/**
		 * Returns whether or not the socket is connected.
		 */
		bool IsConnected() const { return _socket != INVALID_SOCKET; }

		/**
		 * Sends a pre-built MBP message. The message passed MUST have already had
		 * its Build() method called prior to calling this method.
		 *
		 * @return   true - message sent, false - failed to send message, message is invalid, socket is not connected
		 */
		bool SendBuiltMessage(const BaseMessage * message);

		/**
		 * IRunnable methods
		 */
		virtual void Run(const Thread * thread);

		/**
		 * returns the unique session ID of the socket
		 *
		 * @return   sessionID
		 */
		unsigned int GetSessionID() const { return _sessionID; }

		/**
		 * Reads the next message from the socket
		 */
		void ReadMessage();

	protected:
		SOCKET _socket;///< socket handle
		Thread _selectThread;///< thread object used for reading from the socket.
		bool _isListening;///< stores whether or not the socket is a listening socket
		unsigned int _sessionID;///< unique sessionID
		ISocketHandler * _handler;///< event handler
		std::shared_ptr<DataBuffer>	_readBuffer; ///< reusable buffer for reading messages directly from the socket
		std::unique_ptr<BaseMessage> _currentMsg;///< current message read and parsed

	protected:
		/**
		 * Sets socket options. Currently, just receive timeout
		 */
		void SetSockOpts();
	};
}