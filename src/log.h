/*****************************************************************
 * Description log systeam
 * Email huxiaoheigame@gmail.com
 * Created on 2021/07/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN__LOG_H__
#define __TIGERKIN__LOG_H__

#include <stdint.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define TIGERKIN_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level)      \
        tigerkin::LogEventWrap(tigerkin::LogEvent::ptr(new tigerkin::LogEvent(logger, level, __FILE__, __LINE__, 0, tigerkin::getThreadId(), tigerkin::getFiberId(), time(0)))).getSS()

#define TIGERKIN_LOG_DEBUG(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::DEBUG)
#define TIGERKIN_LOG_INFO(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::INFO)
#define TIGERKIN_LOG_WARN(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::WARN)
#define TIGERKIN_LOG_ERROR(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::ERROR)
#define TIGERKIN_LOG_FATAL(logger) TIGERKIN_LOG_LEVEL(logger, tigerkin::LogLevel::FATAL)

#define TIGERKIN_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if (logger->getLevel() <= level)                    \
        tigerkin::LogEventWrap(tigerkin::LogEvent::ptr(new tigerkin::LogEvent(logger, level, __FILE__, __LINE__, 0, tigerkin::getThreadId(), tigerkin::getFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define TIGERKIN_LOG_FMT_DEBUG(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_INFO(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::INFO, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_WARN(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::WARN, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_ERROR(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::ERROR, fmt, __VA_ARGS__)
#define TIGERKIN_LOG_FMT_FATAL(logger, fmt, ...) TIGERKIN_LOG_FMT_LEVEL(logger, tigerkin::LogLevel::FATAL, fmt, __VA_ARGS__)

using namespace std;

namespace tigerkin {

class Logger;

class LogLevel {
   public:
    enum Level { DEBUG = 1,
                 INFO = 2,
                 WARN = 3,
                 ERROR = 4,
                 FATAL = 5 };
    static const std::string toString(LogLevel::Level level);
    static LogLevel::Level fromString(const std::string& str);
};

class LogEvent {
   public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line,
             uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time);
    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    int32_t getElapse() const { return m_elapse; }
    int32_t getThreadId() const { return m_threadId; }
    const std::string& getThreadName() const { return m_threadName; }
    int32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_ss.str(); }
    std::stringstream& getSS() { return m_ss; }
    LogLevel::Level getLevel() const { return m_level; }
    std::shared_ptr<Logger> getLogger() { return m_logger; }

    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

   private:
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    const char* m_file;
    int32_t m_line = 0;
    uint32_t m_elapse = 0;
    int32_t m_threadId = 0;
    std::string m_threadName;
    uint32_t m_fiberId = 0;
    uint64_t m_time;
    std::stringstream m_ss;
};

class LogEventWrap {
   public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const { return m_event; }
    std::stringstream& getSS();

   private:
    LogEvent::ptr m_event;
};

class LogFormatter {
   public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                       LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger,
                         LogLevel::Level level, LogEvent::ptr event);
    bool isError() const { return m_error; }
    const std::string getPattern() const { return m_pattern; }

   public:
    class FormatItem {
       public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem(){};
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
                            LogLevel::Level level, LogEvent::ptr event) = 0;
    };

   private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    void init();
    bool m_error = false;
};

class LogAppender {
   public:
    typedef std::shared_ptr<LogAppender> ptr;

    virtual ~LogAppender() {}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event) = 0;
    void setFormate(LogFormatter::ptr val) { m_formatter = val; }
    LogFormatter::ptr getFormate() const { return m_formatter; }

   protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

class StdOutLogAppend : public LogAppender {
   public:
    typedef std::shared_ptr<StdOutLogAppend> ptr;

    void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
             LogEvent::ptr event) override;
};

class FileLogAppend : public LogAppender {
   public:
    typedef std::shared_ptr<FileLogAppend> ptr;

    FileLogAppend(const std::string& filename);
    bool reopen();
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
             LogEvent::ptr event) override;

   private:
    std::string m_filename;
    std::ofstream m_filestream;
};

class Logger : public std::enable_shared_from_this<Logger> {
   public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name = "root");
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void setLevel(const LogLevel::Level level) { m_level = level; }
    LogLevel::Level getLevel() const { return m_level; }
    std::string getName() const { return m_name; }
    void log(LogLevel::Level level, const LogEvent::ptr event);
    void debug(const LogEvent::ptr event);
    void info(const LogEvent::ptr event);
    void warn(const LogEvent::ptr event);
    void error(const LogEvent::ptr event);
    void fatal(const LogEvent::ptr event);

   private:
    std::string m_name;
    LogLevel::Level m_level;
    std::list<LogAppender::ptr> m_appenders;
    LogFormatter::ptr m_formatter;
};

}  // namespace tigerkin

#endif  // !__TIGERKIN__LOG_H__