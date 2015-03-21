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
		_isListening(false),
		_readBuffer(new DataBuffer(kMBPHeaderSize))
	{
	}

	MemcacheSocket::MemcacheSocket(MemcacheSocket::ISocketHandler * handler) :
		_socket(INVALID_SOCKET),
		_sessionID(GetNextSessionID()),
		_handler(handler),
		_isListening(false),
		_readBuffer(new DataBuffer(kMBPHeaderSize))
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

		_isListening = true;

		//
		// now that the socket is listening let's start up the select thread
		// so that we can accept the incoming connections
		//
		_selectThread.Start(this);

		return true;
	}

	bool MemcacheSocket::Connect(const std::string & address, unsigned short port)
	{
		this->Close();

		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			return false;
		}

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr(address.c_str());
		addr.sin_port = htons(port);

		int res = connect(_socket, (const sockaddr *)&addr, sizeof(addr));
		if (SOCKET_ERROR == res)
		{
			std::cout << "Failed to connect to " << address << ":" << port << std::endl;
			this->Close();
			return false;
		}

		//
		// now that we have connected, start the select thread so that we can listen
		// for incoming messages.
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
		_isListening = false;
	}

	void MemcacheSocket::Run(const Thread * thread)
	{
		while (!thread->ShouldTerminate())
		{
			fd_set readFD;
			timeval val;
			val.tv_sec = 10;
			val.tv_usec = 0;

			FD_ZERO(&readFD);
			FD_SET(_socket, &readFD);

			//
			// 
			int res = select(_socket + 1, &readFD, NULL, NULL, &val);
			if (res > 0)
			{
				if (thread->ShouldTerminate())
				{
					// when TCP sockets close gracefully they will appear as if data was read so if
					// the thread says we should terminate let's do so
					break;
				}

				if (_isListening)
				{
					// todo: call accept
					sockaddr_in addr;
					int addrLen;
					SOCKET socket = accept(_socket, (sockaddr*)&addr, &addrLen);
					if (INVALID_SOCKET != socket)
					{
						_handler->OnAcceptConn(new MemcacheSocket(socket, _handler));
					}
					else
					{
						std::cout << "Failed to accept incoming connection!" << std::endl;
					}
				}
				else
				{
					// there's data to be read!
					this->ReadData();
				}
			}
			else
			{
				// there was an internal socket error in select so he socket
				// has probably been closed abnormally
				break;
			}
		}
	}

	void MemcacheSocket::ReadData()
	{
		int read;
		//
		// read the fixed packet header first so we 
		// can determine the Total Body Length of the message
		// and can read the remainder of it.
		//
		_readBuffer->Reset();

		//
		// read the header
		//
		while (_readBuffer->GetBytesWritten() < kMBPHeaderSize)
		{
			read = recv(_socket, (char *)_readBuffer->GetData(), (kMBPHeaderSize - _readBuffer->GetBytesWritten()), 0);
			if (read != SOCKET_ERROR)
			{
				_readBuffer->MoveWriteForward(read);
			}
			else
			{
				std::cout << "SOCKET_ERROR: " << WSAGetLastError() << std::endl;
				break;
			}
		}

		MessageHeader header;
		size_t parsedBytes = header.ParseFromBuffer(*_readBuffer);
		if (parsedBytes != kMBPHeaderSize)
		{
			std::cout << "WARNING: parsed bytes " << parsedBytes << " != header size " << kMBPHeaderSize << std::endl;
		}

		//
		// resize the read buffer if there's not enough space to read
		// the remainder of this message.
		//
		size_t reqSize = header.TotalBodyLen + kMBPHeaderSize;
		if (_readBuffer->GetBufferSize() < reqSize)
		{
			_readBuffer->Realloc(reqSize);
		}

		//
		// read the remainder of the message into the read buffer
		//
		while (reqSize > _readBuffer->GetBytesWritten())
		{
			read = recv(_socket, (char *)_readBuffer->GetWritePtr(), (reqSize - _readBuffer->GetBytesWritten()), 0);
			if (read != SOCKET_ERROR)
			{
				_readBuffer->MoveWriteForward(read);
			}
			else
			{
				std::cout << "SOCKET_ERROR: " << WSAGetLastError() << std::endl;
				break;
			}
		}


		//
		// now that we have read the entire message into our read buffer, let's
		// figure out what type of message this is and pass it up to our handler
		//
		if (header.Magic == kMagicReq)
		{
			switch (header.Opcode)
			{
			case kOpGet:
				_currentMsg.reset(new GetRequest());
				break;
			case kOpSet:
				_currentMsg.reset(new SetRequest());
				break;
			default:
				break;
			}
		}
		else if (header.Magic == kMagicRes)
		{
			switch (header.Opcode)
			{
			case kOpGet:
				_currentMsg.reset(new GetResponse());
				break;
			case kOpSet:
				_currentMsg.reset(new SetResponse());
				break;
			default:
				break;
			}
		}
		std::unique_ptr<BaseMessage> reply;
		_handler->OnReceivedMessage(_currentMsg.get(), reply);

		if (reply)
		{
			// todo: send reply back to the other end
		}
	}
}