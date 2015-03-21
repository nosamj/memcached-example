#include <algorithm>
#include "DataBuffer.h"

namespace memcache
{
	DataBuffer::DataBuffer()
		: _data(nullptr),
		_bufferSize(0),
		_writeOffset(0),
		_readOffset(0)
	{
	}

	DataBuffer::DataBuffer(size_t len)
		: _data(nullptr),
		_bufferSize(0),
		_writeOffset(0),
		_readOffset(0)
	{
		this->Alloc(len);
	}

	DataBuffer::~DataBuffer()
	{
		this->Cleanup();
	}

	bool DataBuffer::Alloc(size_t len)
	{
		this->Cleanup();

		_data = new unsigned char[len];
		_bufferSize = len;

		return this->IsValid();
	}

	bool DataBuffer::Realloc(size_t len)
	{
		if (this->IsValid())
		{
			//
			// 1. create a new buffer
			// 2. read data from existing buffer into new buffer
			// 3. destroy old buffer.
			// 4. assign new buffer
			//
			unsigned char * temp = new unsigned char[len];
			size_t oldRead = _readOffset;
			this->ResetRead();
			this->ReadBytes(temp, len);

			delete[] _data;
			_data = temp;
			_bufferSize = len;

			// in case the realloc'd size is smaller than where the previous
			// buffer had been written to let's clamp the read and write offsets
			// so that they're not larger than the new buffer size
			_writeOffset = std::min(_writeOffset, _bufferSize);
			_readOffset = std::min(_readOffset, _bufferSize);

			return this->IsValid();
		}
		else
		{
			return this->Alloc(len);
		}
	}

	void DataBuffer::Cleanup()
	{
		delete[] _data;
		_data = nullptr;

		this->Reset();
	}

	unsigned char * DataBuffer::GetWritePtr()
	{
		if (this->GetBytesLeftForWrite() > 0)
		{
			return &_data[_writeOffset];
		}
		return nullptr;
	}

	const unsigned char * DataBuffer::GetWritePtr() const
	{
		if (this->GetBytesLeftForWrite() > 0)
		{
			return &_data[_writeOffset];
		}
		return nullptr;
	}

	unsigned char * DataBuffer::GetReadPtr()
	{
		if (this->GetBytesLeftToRead() > 0)
		{
			return &_data[_readOffset];
		}
		return nullptr;
	}

	const unsigned char * DataBuffer::GetReadPtr() const
	{
		if (this->GetBytesLeftToRead() > 0)
		{
			return &_data[_readOffset];
		}
		return nullptr;
	}

	void DataBuffer::ResetRead()
	{
		_readOffset = 0;
	}

	void DataBuffer::Reset()
	{
		_readOffset = 0;
		_writeOffset = 0;
	}

	unsigned char DataBuffer::ReadByte()
	{
		unsigned char ret = 0;
		if (this->IsValid() && ((_readOffset + 1) < _bufferSize))
		{
			ret = _data[_readOffset++];
		}
		return ret;
	}

	bool DataBuffer::WriteByte(unsigned char byte)
	{
		if (this->IsValid() && ((_writeOffset + 1) < _bufferSize))
		{
			_data[_writeOffset++] = byte;

			return true;
		}
		return false;
	}

	unsigned short DataBuffer::ReadShort()
	{
		unsigned short ret = 0;
		if (this->IsValid() && ((_readOffset + 2) < _bufferSize))
		{
			ret = (_data[_readOffset] << 8) | _data[_readOffset + 1];

			this->MoveReadForward(2);
		}
		return ret;
	}

	bool DataBuffer::WriteShort(unsigned short val)
	{
		if (this->IsValid() && ((_writeOffset + 2) < _bufferSize))
		{
			_data[_writeOffset++] = (val >> 8);
			_data[_writeOffset++] = val & 0xFF;

			return true;
		}
		return false;
	}

	unsigned int DataBuffer::ReadInt()
	{
		unsigned int ret = 0;
		if (this->IsValid() && ((_readOffset + 4) < _bufferSize))
		{
			ret = ((_data[_readOffset] << 24) | (_data[_readOffset + 1] << 16) | (_data[_readOffset + 2] << 8) | _data[_readOffset + 3]);

			this->MoveReadForward(4);
		}
		return ret;
	}

	bool DataBuffer::WriteInt(unsigned int val)
	{
		if (this->IsValid() && ((_writeOffset + 4) < _bufferSize))
		{
			_data[_writeOffset++] = (val >> 24) & 0xFF;
			_data[_writeOffset++] = (val >> 16) & 0xFF;
			_data[_writeOffset++] = (val >> 8) & 0xFF;
			_data[_writeOffset++] = val & 0xFF;

			return true;
		}
		return false;
	}

	unsigned long long DataBuffer::ReadLongLong()
	{
		unsigned long long ret = 0;
		if (this->IsValid() && ((_readOffset + 8) < _bufferSize))
		{
			ret = ((unsigned long long)_data[_readOffset++] << 56);
			ret |= ((unsigned long long)_data[_readOffset++] << 48);
			ret |= ((unsigned long long)_data[_readOffset++] << 40);
			ret |= ((unsigned long long)_data[_readOffset++] << 32);
			ret |= (_data[_readOffset++] << 24);
			ret |= (_data[_readOffset++] << 16);
			ret |= (_data[_readOffset++] << 8);
			ret |= _data[_readOffset++];
		}
		return ret;
	}

	bool DataBuffer::WriteLongLong(unsigned long long val)
	{
		if (this->IsValid() && ((_writeOffset + 8) < _bufferSize))
		{
			_data[_writeOffset++] = (val >> 56) & 0xFF;
			_data[_writeOffset++] = (val >> 48) & 0xFF;
			_data[_writeOffset++] = (val >> 40) & 0xFF;
			_data[_writeOffset++] = (val >> 32) & 0xFF;
			_data[_writeOffset++] = (val >> 24) & 0xFF;
			_data[_writeOffset++] = (val >> 16) & 0xFF;
			_data[_writeOffset++] = (val >> 8) & 0xFF;
			_data[_writeOffset++] = val & 0xFF;

			return true;
		}
		return false;
	}

	size_t DataBuffer::ReadBytes(void * dest, size_t numToRead)
	{
		size_t numRead = std::min(numToRead, this->GetBytesLeftToRead());
		if (numRead > 0)
		{
			memcpy(dest, _data, numRead);
		}
		return numRead;
	}

	size_t DataBuffer::ReadAllBytes(void * dest)
	{
		size_t ret = this->GetBytesWritten();
		if (ret > 0)
		{
			memcpy(dest, _data, ret);
		}
		return ret;
	}

	size_t DataBuffer::WriteBytes(const void * data, size_t numToWrite)
	{
		size_t numWritten = std::min(numToWrite, this->GetBytesLeftForWrite());
		if (numWritten > 0)
		{
			memcpy(&_data[_writeOffset], data, numWritten);

			this->MoveWriteForward(numWritten);
		}
		return numWritten;
	}

	size_t DataBuffer::GetBytesWritten() const
	{
		return _writeOffset;
	}

	size_t DataBuffer::GetBytesRead() const
	{
		return _readOffset;
	}

	size_t DataBuffer::GetBytesLeftToRead() const
	{
		if (_writeOffset > _readOffset)
		{
			return (_writeOffset - _readOffset);
		}
		return 0;
	}

	size_t DataBuffer::GetBytesLeftForWrite() const
	{
		if (_bufferSize > _writeOffset)
		{
			return (_bufferSize - _writeOffset);
		}
		return 0;
	}

	size_t DataBuffer::MoveReadForward(size_t len)
	{
		return _readOffset = std::min((_readOffset + len), _writeOffset);
	}

	size_t DataBuffer::MoveReadBackward(size_t len)
	{
		_readOffset -= len;
		if (_readOffset > _writeOffset)
		{
			// we wrapped so set to 0
			_readOffset = 0;
		}
		return _readOffset;
	}

	size_t DataBuffer::MoveWriteForward(size_t len)
	{
		return _writeOffset = std::min((_writeOffset + len), _bufferSize);
	}

	size_t DataBuffer::MoveWriteBackward(size_t len)
	{
		_writeOffset -= len;
		if (_writeOffset > _bufferSize)
		{
			// we wrapped so set to 0
			_writeOffset = 0;
		}
		return _writeOffset;
	}

	DataBuffer & DataBuffer::operator=(const DataBuffer & rhs)
	{
		if (this != &rhs)
		{
			this->Alloc(rhs.GetBufferSize());

			memcpy(this->GetData(), rhs.GetData(), rhs.GetBufferSize());

			_writeOffset = rhs._writeOffset;
			_readOffset = rhs._readOffset;
		}

		return *this;
	}
}