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
	}

	BaseMessage::~BaseMessage()
	{
		_buffer.reset();
	}

	const MessageHeader & BaseMessage::GetHeader() const
	{
		return _baseHeader;
	}

	void BaseMessage::SetHeader(const MessageHeader & header)
	{
		_baseHeader = header;
	}
}