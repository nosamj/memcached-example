// MemcacheServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "MemcacheServer.h"

using namespace memcache;

int _tmain(int argc, _TCHAR* argv[])
{
	std::unique_ptr<MemcacheServer> server(new MemcacheServer());

	//
	// perform WSA initialization
	//

	if (!server->Start())
	{
		std::cout << "Failed to start server!" << std::endl;
		return -1;
	}

	std::string command;
	while (command != "shutdown")
	{
		// todo: print commands
		std::cout << "Enter command or \"shutdown\" to stop the application: ";
		std::cin >> command;
	}


	server.reset();

	return 0;
}

