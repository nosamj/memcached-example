// MemcacheClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>


int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	//
	// perform WSA initialization
	//
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NO_ERROR) 
	{
		std::cout << "WSAStartup failed with error: " << res << std::endl;
		return -1;
	}

	WSACleanup();

	return 0;
}

