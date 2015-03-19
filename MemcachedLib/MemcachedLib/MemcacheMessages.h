#pragma once
#ifndef MEMACHED_MEMCACHEDMESSAGE_H_
#define MEMACHED_MEMCACHEDMESSAGE_H_

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
		unsigned int Opaque;
		unsigned long long CAS;
	}MessageHeader;

	class Message
	{
	public:
		Message();
		virtual ~Message();

		bool AllocateMessage(unsigned int totalBodyLen);

		const MessageHeader & GetHeader() const;
		void SetHeader(MessageHeader & header);

		virtual bool BuildMessage();

		unsigned char * GetKeyData();
		const unsigned char * GetKeyData() const;

		unsigned char * GetExtrasData();
		const unsigned char * GetExtrasData() const;

		unsigned char * GetValueData();
		const unsigned char * GetValueData() const;

		unsigned char GetMagic() const;
		void SetMagic(unsigned char val);

		unsigned char GetOpcode() const;
		void SetOpcode(unsigned char val);

		unsigned short GetKeyLen() const;
		void SetKeyLen(unsigned short val);

		unsigned char GetExtrasLen() const;
		void SetExtrasLen(unsigned char val);

		unsigned char GetDataType() const;
		// not going to provide a setter for DataType since it has only one value of 0

		unsigned short GetStatus() const;
		void SetStatus(unsigned short val);

		unsigned int GetOpaque() const;
		void SetOpaque(unsigned int val);

		unsigned long long GetCAS() const;
		void SetCAS(unsigned long long val);
	};

	class GetRequest : public Message
	{
	public:
		GetRequest();
	};

	class GetResponse : public Message
	{
	public:
		GetResponse();
	};

	class SetRequest : public Message
	{
	public:
		SetRequest();
	};

	class SetResponse : public Message
	{
	public:
		SetResponse();
	};
}
#endif