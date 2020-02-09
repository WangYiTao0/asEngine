#pragma once

#include <string>
#include <set>

namespace as
{
	class asStartupArguments
	{
	public:
		static std::set<std::string> params;

		static void Parse(const wchar_t* args);
		static bool HasArgument(const std::string& value);
	};
}

