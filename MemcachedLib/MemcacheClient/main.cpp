// MemcacheClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "MemcacheClient.h"

using namespace memcache;

int _tmain(int argc, _TCHAR* argv[])
{
	MemcacheClient client;
	std::string serverAddress;
	unsigned short serverPort;
	WSADATA wsaData;

	if (argc < 3)
	{
		std::cout << "Not enough parameters!" << std::endl;
		return -1;
	}

	serverAddress = argv[1];
	serverPort = (unsigned short)atoi(argv[2]);

	//
	// perform WSA initialization
	//
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NO_ERROR) 
	{
		std::cout << "WSAStartup failed with error: " << res << std::endl;
		return -1;
	}

	if (client.Connect(serverAddress, serverPort))
	{
		std::string command;
		std::string key;
		std::string value;
		while (command != "quit")
		{
			// todo: print commands
			std::cin >> command;
			if (command == "set")
			{
				std::cout << "Enter Key:";
				std::cin >> key;
				std::cout << "Enter Value:";
				std::cin >> value;

				DataBuffer buffer(value.length());
				buffer.WriteBytes(value.c_str(), value.length());
				if (client.Set(key, 0xDEADBEEF, buffer))
				{
					std::cout << "Set successful!" << std::endl;
				}
				else
				{
					std::cout << "Set failed!" << std::endl;
				}
			}
			else if (command == "get")
			{
				std::cout << "Enter Key:";
				std::cin >> key;
				DataBuffer valueBuffer;
				unsigned int flags;

				if (client.Get(key, valueBuffer, flags))
				{
					std::cout << "Get Successful! Value:" << std::string((const char *)valueBuffer.GetData(), valueBuffer.GetBytesWritten()) << std::endl;
				}
				else
				{
					std::cout << "Get Failed!" << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "Connect failed!" << std::endl;
	}

	client.Disconnect();

	WSACleanup();

	return 0;
}

