#pragma once
#ifndef MEMCACHE_DATABUFFER_H_
#define MEMCACHE_DATABUFFER_H_

namespace memcache
{
	class DataBuffer
	{
	public:
		DataBuffer();
		DataBuffer(const DataBuffer & rhs);
		DataBuffer(size_t len);
		~DataBuffer();

		bool Alloc(size_t len);

		/**
		 * Resizes the internal buffer to the new size while maintaining
		 * whatever data has been written to the buffer prior to this call.
		 */
		bool Realloc(size_t len);

		void Cleanup();

		bool IsValid() const { return this->GetData() != nullptr; }

		size_t GetBufferSize() const { return _bufferSize; }
		unsigned char * GetData() { return _data; }
		const unsigned char * GetData() const { return _data; }

		unsigned char * GetWritePtr();
		const unsigned char * GetWritePtr() const;

		unsigned char * GetReadPtr();
		const unsigned char * GetReadPtr() const;

		/**
		 * Resets the read offset to 0
		 */
		void ResetRead();

		/**
		 * Resets both the read and write offset to 0
		 */
		void Reset();

		unsigned char ReadByte();
		bool WriteByte(unsigned char byte);

		unsigned short ReadShort();
		bool WriteShort(unsigned short val);

		unsigned int ReadInt();
		bool WriteInt(unsigned int val);

		unsigned long long ReadLongLong();
		bool WriteLongLong(unsigned long long val);

		size_t ReadBytes(void * dest, size_t numToRead);
		size_t ReadAllBytes(void * dest);
		size_t WriteBytes(const void * data, size_t numToWrite);

		size_t GetBytesLeftToRead() const;
		size_t GetBytesLeftForWrite() const;

		size_t GetBytesWritten() const;
		size_t GetBytesRead() const;

		size_t MoveReadForward(size_t len);
		size_t MoveReadBackward(size_t len);

		size_t MoveWriteForward(size_t len);
		size_t MoveWriteBackward(size_t len);

		DataBuffer & operator=(const DataBuffer & rhs);

	protected:
		unsigned char * _data;
		size_t _bufferSize;
		size_t _writeOffset;
		size_t _readOffset;
	};
}
#endif