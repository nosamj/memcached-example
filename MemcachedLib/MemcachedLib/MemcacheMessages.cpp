#include "MemcacheMessages.h"
#include <string>

namespace memcache
{

	/**
	 * MessageHeader implementation
	 */
	MessageHeader::MessageHeader()
	{
		memset(this, 0, sizeof(MessageHeader));
	}

	size_t MessageHeader::ParseFromBuffer(DataBuffer & buffer)
	{
		size_t start = buffer.GetBytesLeftToRead();
		if (buffer.GetBytesLeftToRead() >= kMBPHeaderSize)
		{
			this->Magic = buffer.ReadByte();
			this->Opcode = buffer.ReadByte();
			this->KeyLen = buffer.ReadShort();
			this->ExtrasLen = buffer.ReadByte();
			this->DataType = buffer.ReadByte();
			this->Status = buffer.ReadShort();
			this->TotalBodyLen = buffer.ReadInt();
			this->Opaque = buffer.ReadInt();
			this->CAS = buffer.ReadLongLong();
		}
		return start - buffer.GetBytesLeftToRead();
	}

	size_t MessageHeader::WriteToBuffer(DataBuffer & buffer)
	{
		size_t start = buffer.GetBytesWritten();
		if (buffer.GetBytesLeftForWrite() >= kMBPHeaderSize)
		{
			buffer.WriteByte(this->Magic);
			buffer.WriteByte(this->Opcode);
			buffer.WriteShort(this->KeyLen);
			buffer.WriteByte(this->ExtrasLen);
			buffer.WriteByte(this->DataType);
			buffer.WriteShort(this->Status);
			buffer.WriteInt(this->TotalBodyLen);
			buffer.WriteInt(this->Opaque);
			buffer.WriteLongLong(this->CAS);
		}
		return buffer.GetBytesWritten() - start;
	}

	/**
	 * BaseMessage implementation
	 */
	BaseMessage::BaseMessage()
	{
		_buffer.reset(new DataBuffer(2048));
	}

	BaseMessage::BaseMessage(const std::shared_ptr<DataBuffer> & buffer) : _buffer(buffer)
	{
	}

	BaseMessage::~BaseMessage()
	{
		_buffer.reset();
	}

	const MessageHeader & BaseMessage::GetHeader() const
	{
		return _baseHeader;
	}

	MessageHeader & BaseMessage::GetHeader()
	{
		return _baseHeader;
	}

	void BaseMessage::SetHeader(const MessageHeader & header)
	{
		_baseHeader = header;
	}

	void BaseMessage::SetStatus(unsigned short status)
	{
		_baseHeader.Status = status;
	}

	unsigned short BaseMessage::GetStatus() const
	{
		return _baseHeader.Status;
	}

	bool BaseMessage::Parse()
	{
		if (_buffer)
		{
			_buffer->ResetRead();
			if (_baseHeader.ParseFromBuffer(*_buffer.get()) == kMBPHeaderSize)
			{
				bool ret = this->ReadExtras();

				ret &= this->ReadKey();

				ret &= this->ReadValue();

				return ret;
			}
		}
		return false;
	}

	bool BaseMessage::Build()
	{
		if (_buffer)
		{
			_buffer->Reset();
			_buffer->MoveWriteForward(kMBPHeaderSize);
			_baseHeader.TotalBodyLen += _baseHeader.ExtrasLen = this->WriteExtras();
			_baseHeader.TotalBodyLen += _baseHeader.KeyLen = this->WriteKey();
			_baseHeader.TotalBodyLen += this->WriteValue();

			// reset the buffer so we can write the finalized header
			_buffer->Reset();
			if (_baseHeader.WriteToBuffer(*_buffer.get()) == kMBPHeaderSize)
			{
				// move the write forward in the buffer by the total body length
				_buffer->MoveWriteForward(_baseHeader.TotalBodyLen);

				return true;
			}
		}
		return false;
	}

	void BaseMessage::SetKey(const std::string & key)
	{
		_key = key;
	}

	std::string BaseMessage::GetKey() const
	{
		return _key;
	}

	size_t BaseMessage::WriteExtras()
	{
		return 0;
	}

	bool BaseMessage::ReadExtras()
	{
		return true;
	}

	size_t BaseMessage::WriteKey()
	{
		if (_key.empty())
		{
			return _baseHeader.KeyLen = 0;
		}
		else
		{
			_baseHeader.KeyLen = _key.length();
			if (_buffer->GetBytesLeftForWrite() < _key.length())
			{
				_buffer->Realloc(_buffer->GetBufferSize() + _key.length());
			}

			return _buffer->WriteBytes(_key.c_str(), _key.length());
		}
	}

	bool BaseMessage::ReadKey()
	{
		_key.clear();
		if (_baseHeader.KeyLen > 0)
		{
			if (_buffer->GetBytesLeftToRead() >= _baseHeader.KeyLen)
			{
				_key = std::string((const char *)_buffer->GetReadPtr(), _baseHeader.KeyLen);
				_buffer->MoveReadForward(_baseHeader.KeyLen);
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	size_t BaseMessage::WriteValue()
	{
		return 0;
	}

	bool BaseMessage::ReadValue()
	{
		return true;
	}

	/**
	 * GetRequest implementation
	 */
	GetRequest::GetRequest()
	{
		_baseHeader.ExtrasLen = 0;
		_baseHeader.TotalBodyLen = 0;
	}

	GetRequest::GetRequest(const std::shared_ptr<DataBuffer> & buffer) : BaseMessage(buffer)
	{
	}

	bool GetRequest::Parse()
	{
		if (BaseMessage::Parse())
		{
			// per the spec:
			// MUST NOT have extras
			// MUST have key
			// MUST NOT have value
			if (_baseHeader.Magic != kMagicReq)
			{
				return false;
			}

			if (_baseHeader.Opcode != kOpGet)
			{
				return false;
			}

			if (_baseHeader.ExtrasLen > 0)
			{
				return false;
			}

			if (_baseHeader.KeyLen == 0)
			{
				return false;
			}

			if (_baseHeader.TotalBodyLen != _baseHeader.KeyLen)
			{
				return false;
			}
			return true;
		}
		return false;
	}

	bool GetRequest::Build()
	{
		_baseHeader = MessageHeader();
		_baseHeader.Magic = kMagicReq;
		_baseHeader.Opcode = kOpGet;

		// per the spec:
		// MUST NOT have extras
		// MUST have key
		// MUST NOT have value

		return BaseMessage::Build();
	}

	/**
	 * GetResponse implementation
	 */
	GetResponse::GetResponse() : _flags(0)
	{
	}

	GetResponse::GetResponse(const std::shared_ptr<DataBuffer> & buffer) : BaseMessage(buffer), _flags(0)
	{
	}

	bool GetResponse::Parse()
	{
		if (BaseMessage::Parse())
		{
			// per the spec
			// MUST have extras
			// MAY have key
			// MAY have value
			return true;
		}
		return false;
	}

	bool GetResponse::Build()
	{
		_baseHeader.Magic = kMagicRes;
		_baseHeader.Opcode = kOpGet;

		return BaseMessage::Build();
	}

	size_t GetResponse::WriteExtras()
	{
		if (_buffer->WriteInt(_flags))
		{
			return 4;
		}
		else
		{
			return 0;
		}
	}

	bool GetResponse::ReadExtras()
	{
		if (_baseHeader.ExtrasLen == 4)
		{
			_flags = _buffer->ReadInt();
			return true;
		}
		return false;
	}

	size_t GetResponse::WriteValue()
	{
		if (_buffer->GetBytesLeftForWrite() < _value.GetBytesWritten())
		{
			_buffer->Realloc(_buffer->GetBufferSize() + _value.GetBytesWritten());
		}

		return _buffer->WriteBytes(_value.GetData(), _value.GetBytesWritten());
	}

	bool GetResponse::ReadValue()
	{
		size_t valSize = _baseHeader.TotalBodyLen - (_baseHeader.KeyLen + _baseHeader.ExtrasLen);
		if (valSize > 0)
		{
			_value.Alloc(valSize);

			_buffer->ReadBytes(_value.GetData(), valSize);
			_buffer->MoveReadForward(valSize);
			_value.MoveWriteForward(valSize);
		}
		return true;
	}

	/**
	 * SetRequest implementation
	 */
	SetRequest::SetRequest() : BaseMessage(), _flags(0)
	{
	}

	SetRequest::SetRequest(const std::shared_ptr<DataBuffer> & buffer) : BaseMessage(buffer), _flags(0)
	{
	}

	bool SetRequest::Parse()
	{
		if (BaseMessage::Parse())
		{
			// per the spec
			// MUST have extras
			// MUST have key
			// MUST have value
			// Extras: 4 bytes flags, 4 bytes expiration
			if (_baseHeader.ExtrasLen != 8)
			{
				return false;
			}

			if (_baseHeader.KeyLen == 0)
			{
				return false;
			}

			if (_baseHeader.TotalBodyLen <= (unsigned int)(_baseHeader.ExtrasLen + _baseHeader.KeyLen))
			{
				return false;
			}
			return true;
		}
		return false;
	}

	bool SetRequest::Build()
	{
		_baseHeader.Magic = kMagicReq;
		_baseHeader.Opcode = kOpSet;

		return BaseMessage::Build();
	}

	size_t SetRequest::WriteExtras()
	{
		if (_buffer->WriteInt(_flags))
		{
			if (_buffer->WriteInt(_expires))
			{
				return 8;
			}
			else // this would actually be an error because it would produce an invalid set request message
			{
				return 4;
			}
		}
		else
		{
			return 0;
		}
	}

	bool SetRequest::ReadExtras()
	{
		if (_baseHeader.ExtrasLen == 8)
		{
			_flags = _buffer->ReadInt();
			_expires = _buffer->ReadInt();
			return true;
		}
		return false;
	}

	size_t SetRequest::WriteValue()
	{
		if (_buffer->GetBytesLeftForWrite() < _value.GetBytesWritten())
		{
			_buffer->Realloc(_buffer->GetBufferSize() + _value.GetBytesWritten());
		}

		return _buffer->WriteBytes(_value.GetData(), _value.GetBytesWritten());
	}

	bool SetRequest::ReadValue()
	{
		size_t valSize = _baseHeader.TotalBodyLen - (_baseHeader.KeyLen + _baseHeader.ExtrasLen);
		if (valSize > 0)
		{
			_value.Alloc(valSize);

			_buffer->ReadBytes(_value.GetData(), valSize);
			_buffer->MoveReadForward(valSize);
			_value.MoveWriteForward(valSize);
		}
		return true;
	}

	/**
	 * SetResposne implementation
	 */
	SetResponse::SetResponse() : BaseMessage()
	{
	}

	SetResponse::SetResponse(const std::shared_ptr<DataBuffer> & buffer) : BaseMessage(buffer)
	{
	}

	bool SetResponse::Build()
	{
		_baseHeader.Magic = kMagicRes;
		_baseHeader.Opcode = kOpSet;

		return BaseMessage::Build();
	}
}