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
		{
			m_logger->debug(std::forward<TARGS>(args)...);
		}
		break;
		case ELogLevel::Info:
		{
			m_logger->info(std::forward<TARGS>(args)...);
		}
		break;
		case ELogLevel::Warn:
		{
			m_logger->warn(std::forward<TARGS>(args)...);
		}
		break;
		case ELogLevel::Error:
		{
			m_logger->error(std::forward<TARGS>(args)...);
		}
		break;
		case ELogLevel::Fatal:
		{
			m_logger->critical(std::forward<TARGS>(args)...);
			FatalCallback(std::forward<TARGS>(args)...);
		}
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

	std::shared_ptr<spdlog::logger> m_logger;
};

NAMESPACE_XYH_END