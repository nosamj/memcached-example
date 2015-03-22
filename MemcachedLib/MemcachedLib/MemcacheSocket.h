#include <WinSock2.h>
#include <string>
#include "MemcacheDefs.h"
#include "MemcacheMessages.h"
#include "Threading.h"

namespace memcache
{

	class MemcacheSocket : Thread::IRunnable
	{
	public:
		class ISocketHandler
		{
		public:
			virtual void OnAcceptConn(MemcacheSocket * socket) = 0;
			virtual void OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply) = 0;
			virtual void OnSocketClosed(unsigned int sessionID) = 0;
		};

	public:
		MemcacheSocket();
		MemcacheSocket(ISocketHandler * handler);
		MemcacheSocket(SOCKET socket, ISocketHandler * handler);
		~MemcacheSocket();
		bool Listen(unsigned short port, ISocketHandler * handler);
		bool Connect(const std::string & address, unsigned short port);
		void Close();

		/**
		 * IRunnable methods
		 */
		virtual void Run(const Thread * thread);

		unsigned int GetSessionID() const { return _sessionID; }

	protected:
		SOCKET _socket;
		Thread _selectThread;
		bool _isListening;
		unsigned int _sessionID;
		ISocketHandler * _handler;
		std::shared_ptr<DataBuffer>	_readBuffer;
		std::unique_ptr<BaseMessage> _currentMsg;

	protected:
		void ReadData();
	};
}