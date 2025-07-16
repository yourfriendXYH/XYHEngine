#pragma once
#include "../../Common/Common.h"
#include "../../../3rdparty/spdlog/include/spdlog/spdlog.h"
#include <cstdint>

NAMESPACE_XYH_BEGIN

// 日志系统
class LogSystem final
{
public:
	enum class ELogLevel : uint8_t
	{
		Debug = 0,
		Info,
		Warn,
		Error,
		Fatal,	// 致命错误
	};

public:
	LogSystem();
	~LogSystem();

	template<typename... TARGS>
	void Log(ELogLevel logLevel, TARGS&&... args)
	{
		switch (logLevel)
		{
		case ELogLevel::Debug:
			break;
		case ELogLevel::Info:
			break;
		case ELogLevel::Warn:
			break;
		case ELogLevel::Error:
			break;
		case ELogLevel::Fatal:
			break;
		default:
			break;
		}
	}

	template<typename... TARGS>
	void FatalCallback(TARGS&&... args)
	{
		const std::string formatStr = fmt::format(std::forward<TARGS>(args)...);
		throw std::runtime_error(formatStr);
	}

private:

};

NAMESPACE_XYH_END