#include "MemcacheMessages.h"
#include "catch.hpp"

using namespace memcache;
TEST_CASE("MessageHeader Write/Parse")
{
	MessageHeader header, comp;
	DataBuffer buffer(kMBPHeaderSize);

	header.CAS = 0x1122334455667788L;
	header.DataType = 4;
	header.ExtrasLen = 50;
	header.KeyLen = 20;
	header.Magic = 128;
	header.Opaque = 256;
	header.Opcode = 30;
	header.Status = 2048;
	header.TotalBodyLen = 65535;

	REQUIRE(header.WriteToBuffer(buffer) == kMBPHeaderSize);
	REQUIRE(comp.ParseFromBuffer(buffer) == kMBPHeaderSize);

	// now that we have parsed the data into the comparison header
	// let's compare the two to make sure everything matches up
	REQUIRE(header.CAS == comp.CAS);
	REQUIRE(header.DataType == comp.DataType);
	REQUIRE(header.ExtrasLen == comp.ExtrasLen);
	REQUIRE(header.KeyLen == comp.KeyLen);
	REQUIRE(header.Magic == comp.Magic);
	REQUIRE(header.Opaque == comp.Opaque);
	REQUIRE(header.Opcode == comp.Opcode);
	REQUIRE(header.Status == comp.Status);
	REQUIRE(header.TotalBodyLen == comp.TotalBodyLen);
}

TEST_CASE("MessageHeader byte order")
{
	MessageHeader header, comp;
	DataBuffer buffer(kMBPHeaderSize);
	unsigned char byteHeader[kMBPHeaderSize];

	// load some arbitrary header data into the byte header array
	byteHeader[0] = 0x80; // Magic
	comp.Magic = 0x80;

	byteHeader[1] = 0x0A; // Opcode
	comp.Opcode = 0x0A;

	byteHeader[2] = 0x11; // Key Length
	byteHeader[3] = 0x22;
	comp.KeyLen = 0x1122;

	byteHeader[4] = 0x05; // Extras length
	comp.ExtrasLen = 0x05;

	byteHeader[5] = 0x10; // data type
	comp.DataType = 0x10;

	byteHeader[6] = 0x33; // Status
	byteHeader[7] = 0x44;
	comp.Status = 0x3344;

	byteHeader[8] = 0x11; // total body length
	byteHeader[9] = 0x22;
	byteHeader[10] = 0x33;
	byteHeader[11] = 0x44;
	comp.TotalBodyLen = 0x11223344;

	byteHeader[12] = 0x11; // Opaque 
	byteHeader[13] = 0x22;
	byteHeader[14] = 0x33;
	byteHeader[15] = 0x44;
	comp.Opaque = 0x11223344;

	byteHeader[16] = 0x11; // CAS
	byteHeader[17] = 0x22;
	byteHeader[18] = 0x33;
	byteHeader[19] = 0x44;
	byteHeader[20] = 0x55;
	byteHeader[21] = 0x66;
	byteHeader[22] = 0x77;
	byteHeader[23] = 0x88;
	comp.CAS = 0x1122334455667788L;

	//
	// write this byte array to the buffer
	//
	REQUIRE(buffer.WriteBytes(byteHeader, kMBPHeaderSize) == kMBPHeaderSize);

	//
	// parse this data into the unused header
	//
	REQUIRE(header.ParseFromBuffer(buffer) == kMBPHeaderSize);

	//
	// now compare the two headers
	//
	REQUIRE(header.CAS == comp.CAS);
	REQUIRE(header.DataType == comp.DataType);
	REQUIRE(header.ExtrasLen == comp.ExtrasLen);
	REQUIRE(header.KeyLen == comp.KeyLen);
	REQUIRE(header.Magic == comp.Magic);
	REQUIRE(header.Opaque == comp.Opaque);
	REQUIRE(header.Opcode == comp.Opcode);
	REQUIRE(header.Status == comp.Status);
	REQUIRE(header.TotalBodyLen == comp.TotalBodyLen);
}

TEST_CASE("GetRequest successful parse")
{
	std::unique_ptr<GetRequest> req;
	std::shared_ptr<DataBuffer> buffer(new DataBuffer());
	unsigned char byteArray[] = {
		kMagicReq, // magic
		kOpGet, // opcode
		0, 8, // key length
		0, // extras length
		0, // data type
		0, 0, // reserved
		0, 0, 0, 8, // total body length
		0, 0, 0, 0, // opaque
		0, 0, 0, 0, 0, 0, 0, 0, // CAS
		'm', 'e', 'm', 'c', 'a', 'c', 'h', 'e' // key data
	};

	buffer->Alloc(sizeof(byteArray));
	buffer->WriteBytes(byteArray, sizeof(byteArray));

	req.reset(new GetRequest(buffer));
	REQUIRE(req->Parse());
	REQUIRE(req->GetKey() == "memcache");
}

TEST_CASE("GetRequest failed parse")
{
	std::unique_ptr<GetRequest> req;
	std::shared_ptr<DataBuffer> buffer(new DataBuffer());
	unsigned char byteArray[] = {
		kMagicRes, // magic
		kOpGet, // opcode
		0, 8, // key length
		0, // extras length
		0, // data type
		0, 0, // reserved
		0, 0, 0, 8, // total body length
		0, 0, 0, 0, // opaque
		0, 0, 0, 0, 0, 0, 0, 0, // CAS
		'm', 'e', 'm', 'c', 'a', 'c', 'h', 'e' // key data
	};
	buffer->Alloc(sizeof(byteArray));
	buffer->WriteBytes(byteArray, sizeof(byteArray));

	// this should fail because the magic number if wrong
	req.reset(new GetRequest(buffer));
	REQUIRE_FALSE(req->Parse());

	// restore magic number and change op code
	byteArray[0] = kMagicReq;
	byteArray[1] = kOpSet;
	buffer->Reset();
	buffer->WriteBytes(byteArray, sizeof(byteArray));

	// this should fail because the op code is incorrect
	req.reset(new GetRequest(buffer));
	REQUIRE_FALSE(req->Parse());

	// restore op code and set the key length to 0
	byteArray[1] = kOpGet;
	byteArray[2] = 0;
	byteArray[3] = 0;
	buffer->Reset();
	buffer->WriteBytes(byteArray, sizeof(byteArray));

	// this should fail because the key length is 0
	req.reset(new GetRequest(buffer));
	REQUIRE_FALSE(req->Parse());
}

TEST_CASE("GetRequest build")
{
	std::string key = "memcache";
	GetRequest req;
	req.SetKey(key);

	REQUIRE(req.Build());
	MessageHeader header = req.GetHeader();
	REQUIRE(header.Magic == kMagicReq);
	REQUIRE(header.ExtrasLen == 0);
	REQUIRE(header.Opcode == kOpGet);
	REQUIRE(header.KeyLen == key.length());
}

TEST_CASE("GetResponse successful parse")
{
	std::shared_ptr<DataBuffer> buffer(new DataBuffer());
	std::unique_ptr<GetResponse> res;
	unsigned char byteArray[] = {
		kMagicRes, // magic
		kOpGet, // opcode
		0, 8, // key length
		4, // extras length
		0, // data type
		0, 1, // status (Not found)
		0, 0, 0, 16, // total body length
		0, 0, 0, 0, // opaque
		0, 0, 0, 0, 0, 0, 0, 0, // CAS
		0xDE, 0xAD, 0xBE, 0xEF, // flags
		'm', 'e', 'm', 'c', 'a', 'c', 'h', 'e', // key data
		'e', 'x', 'a', 'm' // value data
	};

	buffer->Alloc(sizeof(byteArray));
	buffer->WriteBytes(byteArray, sizeof(byteArray));

	res.reset(new GetResponse(buffer));
	REQUIRE(res->Parse());
	REQUIRE(res->GetFlags() == 0xDEADBEEF);
	DataBuffer & value = res->GetValue();
	REQUIRE(value.GetBytesWritten() == 4);
	unsigned char valArray[] = { 'e', 'x', 'a', 'm' };
	REQUIRE(memcmp(value.GetData(), valArray, 4) == 0);
	REQUIRE(res->GetStatus() == kResKeyNotFound);
}

TEST_CASE("GetResponse build")
{
	GetResponse res;
	std::string key = "memcache";
	unsigned int flags = 0xDEADBEEF;
	DataBuffer valBuffer;
	unsigned char valArray[] = { 'e', 'x', 'a', 'm' };
	valBuffer.Alloc(sizeof(valArray) * 512);
	for (int i = 0; i < 512; ++i)
	{
		valBuffer.WriteBytes(valArray, sizeof(valArray));
	}

	res.SetKey(key);
	res.SetFlags(flags);
	res.SetValue(valBuffer);

	REQUIRE(res.Build());
	MessageHeader header = res.GetHeader();
	REQUIRE(header.Magic == kMagicRes);
	REQUIRE(header.Opcode == kOpGet);
	REQUIRE(header.ExtrasLen == 4);
	REQUIRE(header.KeyLen == key.length());
	REQUIRE(header.TotalBodyLen == (valBuffer.GetBytesWritten() + header.KeyLen + header.ExtrasLen));
}