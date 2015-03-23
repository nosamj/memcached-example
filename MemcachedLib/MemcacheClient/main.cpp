// MemcacheClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "MemcacheClient.h"

using namespace memcache;

void PrintUsage()
{
	std::cout << std::endl << "Usage: MemcacheClient [server_ip] [server_port]" << std::endl;
	std::cout << "server_ip: IP address of the server to connect to." << std::endl;
	std::cout << "server_port: port on which the server is listening." << std::endl;
}

void PrintCommands()
{
	std::cout << std::endl << "Supported Commands:" << std::endl;
	std::cout << "quit: Terminates the application." << std::endl;
	std::cout << "set: sets the provided value of the provided key on the connected server." << std::endl;
	std::cout << "get: gets the value of the provided key." << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	MemcacheClient client;
	std::string serverAddress;
	unsigned short serverPort;
	WSADATA wsaData;

	if (argc > 1)
	{
		std::string arg = argv[1];
		if ((arg.find("help") != std::string::npos) || (arg.find("?") != std::string::npos))
		{
			PrintUsage();
			return 0;
		}
	}

	if (argc < 3)
	{
		std::cout << "Not enough parameters!" << std::endl;
		PrintUsage();
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
		std::string flagsStr;
		PrintCommands();
		while (command != "quit")
		{
			std::cout << std::endl << "Enter command:";
			std::cin >> command;
			if (command == "set")
			{
				std::cout << "Enter Key:";
				std::cin >> key;
				std::cout << "Enter Flags";
				std::cin >> flagsStr;
				std::cout << "Enter Value:";
				std::cin >> value;

				DataBuffer buffer(value.length());
				buffer.WriteBytes(value.c_str(), value.length());
				if (client.Set(key, atoi(flagsStr.c_str()), buffer))
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
					std::cout << "Flags:" << flags << " Value:" << std::string((const char *)valueBuffer.GetData(), valueBuffer.GetBytesWritten()) << std::endl;
				}
				else
				{
					std::cout << "Get Failed!" << std::endl;
				}
			}
			else if ((command == "help") || (command.find("?") != std::string::npos))
			{
				PrintCommands();
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

