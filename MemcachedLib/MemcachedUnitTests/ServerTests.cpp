#include "MemcacheServer.h"
#include "catch.hpp"

using namespace memcache;

class MemcacheClient
{
public:
	MemcacheClient(MemcacheSocket::ISocketHandler * handler) : _handler(handler)
	{
	}

	bool SetData(const std::string & key, const DataBuffer & value)
	{
		SetRequest req;
		std::unique_ptr<BaseMessage> reply;
		req.SetKey(key);
		req.SetFlags(0xDEADBEEF);
		req.SetValue(value);
		req.Build();
		_handler->OnReceivedMessage(&req, reply);

		return (reply && reply->Build() && (reply->GetStatus() == kResNoError));
	}

	DataBuffer GetData(const std::string key)
	{
		GetRequest req;
		std::unique_ptr<BaseMessage> reply;
		req.SetKey(key);
		req.Build();

		_handler->OnReceivedMessage(&req, reply);

		if (reply && reply->Build())
		{
			return static_cast<GetResponse*>(reply.get())->GetValue();
		}
		else
		{
			return DataBuffer();
		}
	}

protected:
	MemcacheSocket::ISocketHandler * _handler;
};

TEST_CASE("Server Simple set/get")
{
	MemcacheServer server;
	MemcacheClient client(&server);
	DataBuffer value1, value2;

	REQUIRE(server.Start(0));

	value1.Alloc(1024);
	value2.Alloc(1024);
	for (int i = 0; i < 256; ++i)
	{
		value1.WriteInt(0xDEADBEEF);
		value2.WriteInt(0xCAFEBABE);
	}

	REQUIRE(client.SetData("value1", value1));
	REQUIRE(client.SetData("value2", value2));

	DataBuffer comp1 = client.GetData("value1");
	REQUIRE(comp1.GetBytesWritten() == value1.GetBytesWritten());
	REQUIRE(memcmp(comp1.GetData(), value1.GetData(), value1.GetBytesWritten()) == 0);

	DataBuffer comp2 = client.GetData("value2");
	REQUIRE(comp2.GetBytesWritten() == value2.GetBytesWritten());
	REQUIRE(memcmp(comp2.GetData(), value2.GetData(), value2.GetBytesWritten()) == 0);

	server.Shutdown();
}