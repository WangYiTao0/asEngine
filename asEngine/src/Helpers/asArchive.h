#pragma once
#include "CommonInclude.h"
#include <stdint.h>

#include <string>
#include <vector>

class asArchive  //dang an
{
private:
	uint64_t version = 0;
	bool readMode = false;
	size_t pos = 0;
	uint8_t* DATA = nullptr;
	size_t dataSize = 0;

	std::string fileName; // save to this file on closing if not empty

	void CreateEmpty();

public:
	// Create empty arhive for writing
	asArchive();
	// Create archive and link to file
	asArchive(const std::string& fileName, bool readMode = true);
	~asArchive();

	const uint8_t* GetData() const { return DATA; }
	size_t GetSize() const { return pos; }
	uint64_t GetVersion() const { return version; }
	bool IsReadMode() const { return readMode; }
	void SetReadModeAndResetPos(bool isReadMode);
	bool IsOpen();
	void Close();
	bool SaveFile(const std::string& fileName);
	std::string GetSourceDirectory() const;
	std::string GetSourceFileName() const;

	// It could be templated but we have to be extremely careful of different datasizes on different platforms
	// because serialized data should be interchangeable!
	// So providing exact copy operations for exact types enforces platform agnosticism

	// Write operations
	inline asArchive& operator<<(bool data)
	{
		_write((uint32_t)(data ? 1 : 0));
		return *this;
	}
	inline asArchive& operator<<(char data)
	{
		_write((int8_t)data);
		return *this;
	}
	inline asArchive& operator<<(unsigned char data)
	{
		_write((uint8_t)data);
		return *this;
	}
	inline asArchive& operator<<(int data)
	{
		_write((int64_t)data);
		return *this;
	}
	inline asArchive& operator<<(unsigned int data)
	{
		_write((uint64_t)data);
		return *this;
	}
	inline asArchive& operator<<(long data)
	{
		_write((int64_t)data);
		return *this;
	}
	inline asArchive& operator<<(unsigned long data)
	{
		_write((uint64_t)data);
		return *this;
	}
	inline asArchive& operator<<(long long data)
	{
		_write((int64_t)data);
		return *this;
	}
	inline asArchive& operator<<(unsigned long long data)
	{
		_write((uint64_t)data);
		return *this;
	}
	inline asArchive& operator<<(float data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(double data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMFLOAT2& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMFLOAT3& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMFLOAT4& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMFLOAT3X3& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMFLOAT4X3& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMFLOAT4X4& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMUINT2& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMUINT3& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const XMUINT4& data)
	{
		_write(data);
		return *this;
	}
	inline asArchive& operator<<(const std::string& data)
	{
		uint64_t len = (uint64_t)(data.length() + 1); // +1 for the null-terminator
		_write(len);
		_write(*data.c_str(), len);
		return *this;
	}
	template<typename T>
	inline asArchive& operator<<(const std::vector<T>& data)
	{
		// Here we will use the << operator so that non-specified types will have compile error!
		(*this) << data.size();
		for (const T& x : data)
		{
			(*this) << x;
		}
		return *this;
	}

	// Read operations
	inline asArchive& operator >> (bool& data)
	{
		uint32_t temp;
		_read(temp);
		data = (temp == 1);
		return *this;
	}
	inline asArchive& operator >> (char& data)
	{
		int8_t temp;
		_read(temp);
		data = (char)temp;
		return *this;
	}
	inline asArchive& operator >> (unsigned char& data)
	{
		uint8_t temp;
		_read(temp);
		data = (unsigned char)temp;
		return *this;
	}
	inline asArchive& operator >> (int& data)
	{
		int64_t temp;
		_read(temp);
		data = (int)temp;
		return *this;
	}
	inline asArchive& operator >> (unsigned int& data)
	{
		uint64_t temp;
		_read(temp);
		data = (unsigned int)temp;
		return *this;
	}
	inline asArchive& operator >> (long& data)
	{
		int64_t temp;
		_read(temp);
		data = (long)temp;
		return *this;
	}
	inline asArchive& operator >> (unsigned long& data)
	{
		uint64_t temp;
		_read(temp);
		data = (unsigned long)temp;
		return *this;
	}
	inline asArchive& operator >> (long long& data)
	{
		int64_t temp;
		_read(temp);
		data = (long long)temp;
		return *this;
	}
	inline asArchive& operator >> (unsigned long long& data)
	{
		uint64_t temp;
		_read(temp);
		data = (unsigned long long)temp;
		return *this;
	}
	inline asArchive& operator >> (float& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (double& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMFLOAT2& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMFLOAT3& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMFLOAT4& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMFLOAT3X3& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMFLOAT4X3& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMFLOAT4X4& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMUINT2& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMUINT3& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (XMUINT4& data)
	{
		_read(data);
		return *this;
	}
	inline asArchive& operator >> (std::string& data)
	{
		uint64_t len;
		_read(len);
		char* str = new char[(size_t)len];
		memset(str, '\0', (size_t)(sizeof(char) * len));
		_read(*str, len);
		data = std::string(str);
		delete[] str;
		return *this;
	}
	template<typename T>
	inline asArchive& operator >> (std::vector<T>& data)
	{
		// Here we will use the >> operator so that non-specified types will have compile error!
		size_t count;
		(*this) >> count;
		data.resize(count);
		for (size_t i = 0; i < count; ++i)
		{
			(*this) >> data[i];
		}
		return *this;
	}



private:

	// This should not be exposed to avoid misaligning data by mistake
	// Any specific type serialization should be implemented by hand
	// But these can be used as helper functions inside this class

	// Write data using memory operations
	template<typename T>
	inline void _write(const T& data, uint64_t count = 1)
	{
		size_t _size = (size_t)(sizeof(data) * count);
		size_t _right = pos + _size;
		if (_right > dataSize)
		{
			uint8_t* NEWDATA = new uint8_t[_right * 2];
			memcpy(NEWDATA, DATA, dataSize);
			dataSize = _right * 2;
			SAFE_DELETE_ARRAY(DATA);
			DATA = NEWDATA;
		}
		memcpy(reinterpret_cast<void*>((uint64_t)DATA + (uint64_t)pos), &data, _size);
		pos = _right;
	}

	// Read data using memory operations
	template<typename T>
	inline void _read(T& data, uint64_t count = 1)
	{
		memcpy(&data, reinterpret_cast<void*>((uint64_t)DATA + (uint64_t)pos), (size_t)(sizeof(data) * count));
		pos += (size_t)(sizeof(data) * count);
	}
};

