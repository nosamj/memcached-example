// MemcacheServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "MemcacheServer.h"

using namespace memcache;

void PrintUsage()
{
	std::cout << std::endl << "Usage: MemcacheServer (listen_port)" << std::endl;
	std::cout << "listen_port: listen on a port different than the default of 30000" << std::endl;
}

void PrintCommands()
{
	std::cout << std::endl << "Supported Commands:" << std::endl;
	std::cout << "quit: Terminates the application." << std::endl;
	std::cout << "printcache: Dumps the current cache to the console." << std::endl;
	std::cout << "clearcache: Clears the cache of all entries." << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::unique_ptr<MemcacheServer> server(new MemcacheServer());
	unsigned short listenPort = 30000;
	WSADATA wsaData;

	if (argc > 1)
	{
		std::string arg = argv[1];
		if ((arg.find("help") != std::string::npos) || (arg.find("?") != std::string::npos))
		{
			PrintUsage();
			return 0;
		}

		int overridePort = atoi(arg.c_str());
		if (overridePort > 0)
		{
			listenPort = (unsigned short)overridePort;
		}
	}

	//
	// perform WSA initialization
	//
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NO_ERROR) 
	{
		std::cout << "WSAStartup failed with error: " << res << std::endl;
		return -1;
	}

	if (!server->Start(listenPort))
	{
		std::cout << "Failed to start server!" << std::endl;
		return -1;
	}

	std::string command;
	PrintCommands();
	while (command != "quit")
	{
		std::cout << std::endl << "Enter command: ";
		std::cin >> command;
		if (command == "printcache")
		{
			server->PrintDataCache();
		}
		else if (command == "clearcache")
		{
			server->ClearDataCache();
		}
		else if ((command == "help") || (command.find("?") != std::string::npos))
		{
			PrintCommands();
		}
	}

	server.reset();

	WSACleanup();

	return 0;
}

