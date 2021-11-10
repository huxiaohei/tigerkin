#ifndef __TIGERKIN_MACRO_H__
#define __TIGERKIN_MACRO_H__

#include <assert.h>
#include <string.h>

#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
#define TIGERKIN_LIKELY(x) __builtin_expect(!!(x), 1)
#define TIGERKIN_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define TIGERKIN_LICKLY(x) (x)
#define TIGERKIN_UNLICKLY(x) (x)
#endif

#define TIGERKIN_LOG_DEBUG(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::DEBUG)
#define TIGERKIN_LOG_INFO(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::INFO)
#define TIGERKIN_LOG_WARN(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::WARN)
#define TIGERKIN_LOG_ERROR(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::ERROR)
#define TIGERKIN_LOG_FATAL(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::FATAL)

#define TIGERKIN_LOG_FMT_DEBUG(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_INFO(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::INFO, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_WARN(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::WARN, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_ERROR(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::ERROR, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_FATAL(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::FATAL, fmt, __VA_ARGS__)

#define TIGERKIN_LOG_ROOT() tigerkin::SingletonLoggerMgr::GetInstance()->getRoot()
#define TIGERKIN_LOG_NAME(name) tigerkin::SingletonLoggerMgr::GetInstance()->getLogger(#name)

#define TIGERKIN_ASSERT(x)                                                                            \
    if (!(x)) {                                                                                       \
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "\nASSERTION: " #x                           \
                                                      << "\nbacktrace:"                               \
                                                      << tigerkin::BacktraceToString(100, 2, "    "); \
        assert(x);                                                                                    \
    }

#define TIGERKIN_ASSERT2(x, w)                                                                        \
    if (!(x)) {                                                                                       \
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "\nASSERTION: " #x << "    " << w            \
                                                      << "\nbacktrace:\n"                             \
                                                      << tigerkin::BacktraceToString(100, 2, "    "); \
    }

#endif