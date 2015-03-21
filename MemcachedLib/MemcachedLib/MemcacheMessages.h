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
		void SetHeader(const MessageHeader & header);

	protected:
		std::shared_ptr<DataBuffer> _buffer;
		MessageHeader _baseHeader;
	};

	class GetRequest : public BaseMessage
	{
	public:
		GetRequest();
		GetRequest(const std::shared_ptr<DataBuffer> & buffer);
		void SetKey(const std::string & key);
		std::string GetKey() const;

	protected:
		std::string _key;
	};

	class GetResponse : public BaseMessage
	{
	public:
		GetResponse();
		GetResponse(const std::shared_ptr<DataBuffer> & buffer);
	};

	class SetRequest : public BaseMessage
	{
	public:
		SetRequest();
		SetRequest(const std::shared_ptr<DataBuffer> & buffer);
	};

	class SetResponse : public BaseMessage
	{
	public:
		SetResponse();
		SetResponse(const std::shared_ptr<DataBuffer> & buffer);
	};
}
#endif