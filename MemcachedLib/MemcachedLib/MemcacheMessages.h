#pragma once
#ifndef MEMACHED_MEMCACHEMESSAGE_H_
#define MEMACHED_MEMCACHEMESSAGE_H_

#include <memory>
#include <string>
#include "MemcacheDefs.h"
#include "DataBuffer.h"

namespace memcache
{
	typedef struct MessageHeader
	{
		unsigned char Magic;
		unsigned char Opcode;
		unsigned short KeyLen;
		unsigned char ExtrasLen;
		unsigned char DataType;
		unsigned short Status;
		unsigned int TotalBodyLen;
		unsigned int Opaque;
		unsigned long long CAS;

		MessageHeader();
		size_t ParseFromBuffer(DataBuffer & buffer);
		size_t WriteToBuffer(DataBuffer & buffer);
	}MessageHeader;

	class BaseMessage
	{
	public:
		BaseMessage();
		BaseMessage(const std::shared_ptr<DataBuffer> & buffer);
		virtual ~BaseMessage();

		const MessageHeader & GetHeader() const;
		MessageHeader & GetHeader();
		void SetHeader(const MessageHeader & header);
		void SetStatus(unsigned short status);
		unsigned short GetStatus() const;
		virtual bool Parse();
		virtual bool Build();

		std::string GetKey() const;
		void SetKey(const std::string & key);

		const DataBuffer * GetDataBuffer() const { return _buffer.get(); }

	protected:
		std::shared_ptr<DataBuffer> _buffer;
		MessageHeader _baseHeader;
		std::string _key;

	protected:
		virtual size_t WriteExtras();
		virtual bool ReadExtras();
		size_t WriteKey();
		bool ReadKey();
		virtual size_t WriteValue();
		virtual bool ReadValue();
	};

	class GetRequest : public BaseMessage
	{
	public:
		GetRequest();
		GetRequest(const std::shared_ptr<DataBuffer> & buffer);
		virtual bool Parse();
		virtual bool Build();
	};

	class GetResponse : public BaseMessage
	{
	public:
		GetResponse();
		GetResponse(const std::shared_ptr<DataBuffer> & buffer);
		virtual bool Parse();
		virtual bool Build();
		void SetValue(const DataBuffer & value) { _value = value; }
		DataBuffer & GetValue() { return _value; }
		void SetFlags(unsigned int flags) { _flags = flags; }
		unsigned int GetFlags() const { return _flags; }

	protected:
		DataBuffer _value;
		unsigned int _flags;

	protected:
		virtual size_t WriteExtras();
		virtual bool ReadExtras();
		virtual size_t WriteValue();
		virtual bool ReadValue();
	};

	class SetRequest : public BaseMessage
	{
	public:
		SetRequest();
		SetRequest(const std::shared_ptr<DataBuffer> & buffer);
		virtual bool Parse();
		virtual bool Build();
		void SetValue(const DataBuffer & value) { _value = value; }
		DataBuffer & GetValue() { return _value; }
		void SetFlags(unsigned int flags) { _flags = flags; }
		unsigned int GetFlags() const { return _flags; }
		void SetExpiration(unsigned int exp) { _expires = exp; }
		unsigned int GetExpiration() const { return _expires; }

	protected:
		DataBuffer _value;
		unsigned int _flags;
		unsigned int _expires;

	protected:
		virtual size_t WriteExtras();
		virtual bool ReadExtras();
		virtual size_t WriteValue();
		virtual bool ReadValue();
	};

	class SetResponse : public BaseMessage
	{
	public:
		SetResponse();
		SetResponse(const std::shared_ptr<DataBuffer> & buffer);
		virtual bool Build();
	};
}
#endif