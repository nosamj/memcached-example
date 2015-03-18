#pragma once
#ifndef _MEMCACHED_MEMCACHEDDEFS_H_
#define _MEMCACHED_MEMCACHEDDEFS_H_

namespace memcached
{
	static const int kMBPHeaderSize = 24;

	/**
	 * Magic number constants
	 */
	static const unsigned char kMagicReq = 0x80;
	static const unsigned char kMagicRes = 0x81;

	/**
	 * Command Opcode values (only supported codes)
	 */
	static const unsigned char kOpGet = 0x00;
	static const unsigned char kOpSet = 0x01;

	/**
	 * Response status values
	 */
	static const unsigned short kResNoError = 0x0000;
	static const unsigned short kResKeyNotFound = 0x0001;
	static const unsigned short kResKeyExists = 0x0002;
	static const unsigned short kResTooLarge = 0x0003;
	static const unsigned short kResInvalidArgs = 0x0004;
	static const unsigned short kResNotStored = 0x0005;
	static const unsigned short kResNonNum = 0x0006;
	static const unsigned short kResUnknownComm = 0x0081;
	static const unsigned short kResOutOfMem = 0x0082;
}
#endif