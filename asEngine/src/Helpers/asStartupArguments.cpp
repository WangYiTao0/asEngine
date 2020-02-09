#include "aspch.h"
#include "asStartupArguments.h"

#include "Helpers/asHelper.h"

namespace as
{
	std::set<std::string> asStartupArguments::params;

	void asStartupArguments::Parse(const wchar_t* args)
	{
		std::wstring from = args;
		std::string to;
		asHelper::StringConvert(from, to);

		std::istringstream iss(to);

		params =
		{
			std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>{}
		};
	}

	bool asStartupArguments::HasArgument(const std::string& value)
	{
		return params.find(value) != params.end();
	}
}



