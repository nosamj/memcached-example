#pragma once
#ifndef MEMCACHE_DATABUFFER_H_
#define MEMCACHE_DATABUFFER_H_

namespace memcache
{
	class DataBuffer
	{
	public:
		DataBuffer() : _data(nullptr), _dataSize(0) {}
		~DataBuffer()
		{
			this->Cleanup();
		}

		bool Alloc(size_t len)
		{
			this->Cleanup();

			_data = new unsigned char[len];
			_dataSize = len;

			return this->IsValid();
		}

		void Cleanup()
		{
			delete _data;
			_data = nullptr;
		}

		bool IsValid() const { return this->GetData() != nullptr; }

		unsigned char * GetData() { return _data; }
		const unsigned char * GetData() const { return _data; }

		size_t GetDataSize() const { return _dataSize; }

	protected:
		unsigned char * _data;
		size_t _dataSize;
	};
}
#endif