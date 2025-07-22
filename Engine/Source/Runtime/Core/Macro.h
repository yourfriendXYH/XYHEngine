#pragma once

#include <thread>

#include "LogSystem.h"
#include "../Function/GlobalContext.h"


// 日志宏
#define LOG_HELPER(LOG_LEVEL, ...) \
	XYH::g_runtimeGlobalContext.m_pLogSystem->Log(LOG_LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_DEBUG(...) LOG_HELPER(XYH::LogSystem::ELogLevel::Debug, __VA_ARGS__);

#define LOG_INFO(...) LOG_HELPER(XYH::LogSystem::ELogLevel::Info, __VA_ARGS__);

#define LOG_WARN(...) LOG_HELPER(XYH::LogSystem::ELogLevel::Warn, __VA_ARGS__);

#define LOG_ERROR(...) LOG_HELPER(XYH::LogSystem::ELogLevel::Error, __VA_ARGS__);

#define LOG_FATAL(...) LOG_HELPER(XYH::LogSystem::ELogLevel::Fatal, __VA_ARGS__);

// 线程等待
#define THREAD_SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));