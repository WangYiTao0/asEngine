#pragma once

#include <memory>

#include "spdlog/spdlog.h"

namespace asLog {

	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define AS_CORE_TRACE(...)    ::asLog::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AS_CORE_INFO(...)     ::asLog::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AS_CORE_WARN(...)     ::asLog::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AS_CORE_ERROR(...)    ::asLog::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AS_CORE_FATAL(...)    ::asLog::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define AS_TRACE(...)	      ::asLog::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AS_INFO(...)	      ::asLog::Log::GetClientLogger()->info(__VA_ARGS__)
#define AS_WARN(...)	      ::asLog::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AS_ERROR(...)	      ::asLog::Log::GetClientLogger()->error(__VA_ARGS__)
#define AS_FATAL(...)	      ::asLog::Log::GetClientLogger()->fatal(__VA_ARGS__)