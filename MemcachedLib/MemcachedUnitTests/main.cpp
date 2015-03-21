// MemcachedUnitTests.cpp : Defines the entry point for the console application.
//

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	return Catch::Session().run(argc, argv);
}

