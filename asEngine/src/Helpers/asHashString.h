#pragma once
#include "CommonInclude.h"
#include <string>

class asHashString
{
private:
	std::string str;
	size_t hash;
public:
	asHashString(const std::string& value = "") : str(value), hash(std::hash<std::string>{}(value)) {}
	asHashString(const char* value) : asHashString(std::string(value)) {}

	constexpr const std::string& GetString() const { return str; }
	constexpr size_t GetHash() const { return hash; }
};

constexpr bool operator==(const asHashString& a, const asHashString& b)
{
	return a.GetHash() == b.GetHash();
}

namespace std
{
	template <>
	struct hash<asHashString>
	{
		constexpr size_t operator()(const asHashString& k) const
		{
			return k.GetHash();
		}
	};
}
