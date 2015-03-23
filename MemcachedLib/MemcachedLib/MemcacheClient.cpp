#include <iostream>
#include "MemcacheClient.h"

namespace memcache
{
	MemcacheClient::MemcacheClient()
	{
	}

	MemcacheClient::~MemcacheClient()
	{
		this->Disconnect();
	}

	bool MemcacheClient::Connect(const std::string & address, unsigned short port)
	{
		this->Disconnect();

		_socket.reset(new MemcacheSocket(this));

		if (!_socket->Connect(address, port, false))
		{
			this->Disconnect();
			return false;
		}
		return true;
	}

	void MemcacheClient::Disconnect()
	{
		if (_socket)
		{
			_socket->Close();
			_socket.reset();
		}

		_readMsg.reset();
	}

	void MemcacheClient::OnAcceptConn(MemcacheSocket * socket)
	{
		// nothing to do because this won't be called on non listening sockets like ours
	}

	void MemcacheClient::OnReceivedMessage(BaseMessage * message, std::unique_ptr<BaseMessage> & reply)
	{
		MessageHeader header = message->GetHeader();
		if (header.Magic == kMagicRes)
		{
			switch (header.Opcode)
			{
			case kOpGet:
			{
				GetResponse * theRes = static_cast<GetResponse*>(message);
				GetResponse * ourRes = new GetResponse();
				_readMsg.reset(ourRes);
				*ourRes = *theRes;
			}
			break;
			case kOpSet:
			{
				SetResponse * theRes = static_cast<SetResponse*>(message);
				SetResponse * ourRes = new SetResponse();
				_readMsg.reset(ourRes);
				*ourRes = *theRes;
			}
			break;
			default:
				break;
			}
		}
	}

	void MemcacheClient::OnSocketClosed(unsigned int sessionID)
	{
		// nothing to do here since this class only owns one socket.
	}

	bool MemcacheClient::Set(const std::string & key, unsigned int flags, const DataBuffer & value)
	{
		if (_socket && _socket->IsConnected())
		{
			SetRequest theReq;
			theReq.SetKey(key);
			theReq.SetFlags(flags);
			theReq.SetValue(value);
			if (theReq.Build())
			{
				if (_socket->SendBuiltMessage(&theReq))
				{
					_socket->ReadMessage();

					if (_readMsg)
					{
						SetResponse * theRes = dynamic_cast<SetResponse*>(_readMsg.get());
						if (nullptr != theRes)
						{
							bool ret = (theRes->GetStatus() == kResNoError);
							_readMsg.reset();
							return ret;
						}
						else
						{
							std::cout << "Set(): Received message is not of type SetResponse!" << std::endl;
						}
					}
					else
					{
						std::cout << "Set(): No message was received from the remote end!" << std::endl;
					}
				}
				else
				{
					std::cout << "Set(): failed to send the built message!" << std::endl;
				}
			}
			else
			{
				std::cout << "Set(): failed to build the SetRequest!" << std::endl;
			}
		}
		else
		{
			std::cout << "Set(): Socket is NOT connected!" << std::endl;
		}
		return false;
	}

	bool MemcacheClient::Get(const std::string & key, DataBuffer & outValue, unsigned int & outFlags)
	{
		if (_socket && _socket->IsConnected())
		{
			GetRequest theReq;
			theReq.SetKey(key);

			if (theReq.Build())
			{
				if (_socket->SendBuiltMessage(&theReq))
				{
					_socket->ReadMessage();

					if (_readMsg)
					{
						GetResponse * theRes = dynamic_cast<GetResponse*>(_readMsg.get());
						if (nullptr != theRes)
						{
							if (theRes->GetStatus() == kResNoError)
							{
								outValue = theRes->GetValue();
								outFlags = theRes->GetFlags();
								_readMsg.reset();
								return true;
							}
							else
							{
								std::cout << "Get(): GetResponse status is an error: " << StatusToString(theRes->GetStatus()) << std::endl;
								_readMsg.reset();
							}
						}
						else
						{
							std::cout << "Get(): Received message is NOT of type GetResponse!" << std::endl;
						}
					}
					else
					{
						std::cout << "Get(): Failed to receive a valid message!" << std::endl;
					}
				}
				else
				{
					std::cout << "Get(): Failed to send built message to server!" << std::endl;
				}
			}
			else
			{
				std::cout << "Get(): Failed to build the GetRequest message!" << std::endl;
			}
		}
		else
		{
			std::cout << "Get(): Socket is NOT connected!" << std::endl;
		}
		return false;
	}
}