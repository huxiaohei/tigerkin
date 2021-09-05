/*****************************************************************
 * Description log systeam
 * Email huxiaoheigame@gmail.com
 * Created on 2021/07/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "log.h"

#include <stdarg.h>
#include <time.h>

#include <cstdio>

#include "config.h"

namespace tigerkin {

template <>
class LexicalCast<std::string, LoggerDefine> {
   public:
    LoggerDefine operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        LoggerDefine loggerDef;
        try {
            loggerDef.name = node["name"].as<std::string>();
            loggerDef.level = LogLevel::fromString(node["level"].as<std::string>());
            loggerDef.formatter = node["formatter"].as<std::string>();
            for (size_t i = 0; i < node["appenders"].size(); ++i) {
                auto it = node["appenders"][i];
                AppenderDefine appenderDef;
                if (it["type"].as<std::string>() == "FileLogAppender") {
                    appenderDef.type = 2;
                    appenderDef.level = LogLevel::fromString(it["level"].as<std::string>());
                    appenderDef.file = it["file"].as<std::string>();
                } else if (it["type"].as<std::string>() == "StdOutLogAppender") {
                    appenderDef.type = 1;
                    appenderDef.level = LogLevel::fromString(it["level"].as<std::string>());
                } else {
                    throw invalid_argument(v);
                }
                loggerDef.appenders.push_back(appenderDef);
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
        return loggerDef;
    }
};

template <>
class LexicalCast<LoggerDefine, std::string> {
   public:
    std::string operator()(const LoggerDefine &v) {
        std::stringstream ss;
        YAML::Node node;
        try {
            node["name"] = v.name;
            node["level"] = LogLevel::toString(v.level);
            node["formatter"] = v.formatter;
            for (auto it : v.appenders) {
                YAML::Node n;
                n["level"] = LogLevel::toString(it.level);
                if (it.type == 1) {
                    n["type"] = "StdOutLogAppender";
                } else if (it.type == 2) {
                    n["type"] = "FileLogAppender";
                    n["file"] = it.file;
                } else {
                    std::cerr << "type error:" << it.type << std::endl;
                    throw invalid_argument("type error");
                }
                node["appenders"].push_back(n);
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
        ss << node;
        return ss.str();
    }
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line,
                   uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time)
    : m_logger(logger), m_level(level), m_file(file), m_line(line), m_elapse(elapse), m_threadId(threadId), m_fiberId(fiberId), m_time(time) {
}

void LogEvent::format(const char *fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char *fmt, va_list al) {
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

LogEventWrap::LogEventWrap(LogEvent::ptr e)
    : m_event(e) {
}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream &LogEventWrap::getSS() {
    return m_event->getSS();
}

const std::string LogLevel::toString(LogLevel::Level level) {
    switch (level) {
#define XX(name)         \
    case LogLevel::name: \
        return #name;

        XX(UNKNOW);
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::fromString(const std::string &str) {
#define XX(level, v)            \
    if (str == #v) {            \
        return LogLevel::level; \
    }

    XX(UNKNOW, unknow)
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(UNKNOW, UNKNOW);
    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);

    return LogLevel::DEBUG;
#undef XX
}

class MessageFormatItem : public LogFormatter::FormatItem {
   public:
    MessageFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
   public:
    LevelFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << LogLevel::toString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
   public:
    ElapseFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
   public:
    NameFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << logger->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
   public:
    ThreadIdFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << event->getThreadId();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
   public:
    ThreadNameFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << event->getThreadName();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
   public:
    FiberIdFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
   public:
    DateTimeFormatItem(const std::string &fmt = "%Y-%m-%d %H:%M:%S")
        : m_format(fmt) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream &os, Logger::ptr logger,
                LogLevel::Level level, LogEvent::ptr event) {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }

   private:
    std::string m_format;
};

class FileNameFormatItem : public LogFormatter::FormatItem {
   public:
    FileNameFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
   public:
    LineFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger,
                LogLevel::Level level, LogEvent::ptr event) {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
   public:
    NewLineFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
   public:
    StringFormatItem(const std::string &fmt = "")
        : m_string(fmt) {}
    void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) {
        os << m_string;
    }

   private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
   public:
    TabFormatItem(const std::string &fmt = "") {}
    void format(std::ostream &os, Logger::ptr logger,
                LogLevel::Level level, LogEvent::ptr event) {
        os << "\t";
    }

   private:
    std::string m_string;
};

LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern) {
    init();
}

void LogFormatter::init() {
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                ++i;
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;
        std::string str;
        std::string fmt;
        while (n < m_pattern.size()) {
            if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' &&
                                m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1) {
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - "
                      << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                               \
    {                                                                            \
#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

        XX(m, MessageFormatItem),     // m:消息
        XX(p, LevelFormatItem),       // p:日志级别
        XX(r, ElapseFormatItem),      // r:累计毫秒数
        XX(c, NameFormatItem),        // c:日志名称
        XX(t, ThreadIdFormatItem),    // t:线程id
        XX(n, NewLineFormatItem),     // n:换行
        XX(d, DateTimeFormatItem),    // d:时间
        XX(f, FileNameFormatItem),    // f:文件名
        XX(l, LineFormatItem),        // l:行号
        XX(T, TabFormatItem),         // T:Tab
        XX(F, FiberIdFormatItem),     // F:协程id
        XX(N, ThreadNameFormatItem),  // N:线程名称
#undef XX
    };

    for (auto &i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back(
                FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(
                    "<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger,
                                 LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto &i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

std::ostream &LogFormatter::format(std::ostream &ofs,
                                   std::shared_ptr<Logger> logger,
                                   LogLevel::Level level, LogEvent::ptr event) {
    for (auto &i : m_items) {
        i->format(ofs, logger, level, event);
    }
    return ofs;
}

void LogAppender::setFormate(LogFormatter::ptr val) {
    // Prevent other threads from getting the m_formatter's value before the current thread modifying it
    SpinLock::Lock lock(m_lock);
    m_formatter = val;
}

LogFormatter::ptr LogAppender::getFormate() {
    // Prevent other threads from modifying the m_formatter's value before the current thread get it
    SpinLock::Lock lock(m_lock);
    return m_formatter;
}

void StdOutLogAppend::log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                          LogEvent::ptr event) {
    if (level >= m_level) {
        // 1. Prevent other threads from modifying the log format (LogAppender::setFormate) while outputting the log
        // 2. Prevent the A thread from outputting half of a log while the B thread also outputting the log, resulting in an incomplete log
        SpinLock::Lock lock(m_lock);
        std::cout << m_formatter->format(logger, level, event);
    }
}

FileLogAppend::FileLogAppend(const std::string &filename)
    : m_filename(filename) {
    if (!reopen())
        std::cerr << "open log file error: " << filename << std::endl;
}

bool FileLogAppend::reopen() {
    // Prevent the file from being closed or changed while the log is being output
    SpinLock::Lock lock(m_lock);
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::fstream::app);
    return !!m_filestream;
}

void FileLogAppend::log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                        LogEvent::ptr event) {
    if (level >= m_level) {
        // Reopen the file every second to prevent the file from being deleted
        uint64_t now = time(0);
        if (now != m_time) {
            reopen();
            m_time = now;
        }
        // 1. Prevent other threads from modifying the log format (LogAppender::setFormate) while outputting the log
        // 2. Prevent the A thread from outputting half of a log while the B thread also outputting the log, resulting in an incomplete log
        SpinLock::Lock lock(m_lock);
        m_formatter->format(m_filestream, logger, level, event);
    }
}

Logger::Logger(const std::string &name)
    : m_name(name), m_level(LogLevel::DEBUG) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S} %t %N %F [%p] [%c] %f:%l %m%n"));
    isValue = true;
}

Logger::Logger(const LoggerDefine &loggerDefine)
    : m_name(loggerDefine.name), m_level(loggerDefine.level) {
    LogFormatter::ptr formate(new LogFormatter(loggerDefine.formatter));
    if (formate->isError()) {
        isValue = false;
        return;
    }
    for (auto appenderDefine : loggerDefine.appenders) {
        if (appenderDefine.type == 1) {
            LogAppender::ptr appender(new StdOutLogAppend());
            appender->setFormate(formate);
            appender->setLevel(appenderDefine.level);
            addAppender(appender);
            isValue = true;
        } else if (appenderDefine.type == 2) {
            LogAppender::ptr appender(new FileLogAppend(appenderDefine.file));
            appender->setFormate(formate);
            appender->setLevel(appenderDefine.level);
            addAppender(appender);
            isValue = true;
        } else {
            isValue = false;
        }
    }
}

void Logger::addAppender(LogAppender::ptr appender) {
    // Prevent threads from modifying the m_appenders's value when outputting the log
    SpinLock::Lock lock(m_lock);
    if (!appender->getFormate()) {
        appender->setFormate(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    // Prevent threads from modifying the m_appenders's value when outputting the log
    SpinLock::Lock lock(m_lock);
    auto it = m_appenders.begin();
    while (it != m_appenders.end()) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
        it++;
    }
}

void Logger::log(LogLevel::Level level, const LogEvent::ptr event) {
    if (level >= m_level) {
        // Prevent threads from modifying the m_appenders's value when outputting the log
        SpinLock::Lock lock(m_lock);
        auto self = shared_from_this();
        for (auto &i : m_appenders) {
            i->log(self, level, event);
        }
    }
}

void Logger::debug(const LogEvent::ptr event) {
    log(LogLevel::Level::DEBUG, event);
}

void Logger::info(const LogEvent::ptr event) {
    log(LogLevel::Level::INFO, event);
}

void Logger::warn(const LogEvent::ptr event) {
    log(LogLevel::Level::WARN, event);
}

void Logger::error(const LogEvent::ptr event) {
    log(LogLevel::Level::ERROR, event);
}

void Logger::fatal(const LogEvent::ptr event) {
    log(LogLevel::Level::FATAL, event);
}

LoggerMgr::LoggerMgr() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdOutLogAppend));
    m_loggers[m_root->getName()] = m_root;
}

void LoggerMgr::addLogger(Logger::ptr logger) {
    // Prevent other threads from modifying m_loggers's value when adding
    SpinLock::Lock lock(m_lock);
    auto it = m_loggers.find(logger->getName());
    if (it != m_loggers.end()) {
        m_loggers.erase(it->first);
    }
    m_loggers[logger->getName()] = logger;
}

void LoggerMgr::addLoggers(const std::string &cfgPath, const std::string &key) {
    ConfigVar<std::vector<LoggerDefine>>::ptr logCfg = Config::lookup("__logs." + key, (std::vector<LoggerDefine>){}, "logs config");
    YAML::Node root = YAML::LoadFile(cfgPath);
    tigerkin::Config::LoadFromYaml(root, "__logs");
    for (auto it : logCfg->getValue()) {
        Logger::ptr logger(new Logger(it));
        if (logger->getIsValue())
            addLogger(logger);
    }
}

void LoggerMgr::deleteLogger(Logger::ptr logger) {
    // Prevent other threads from modifying m_loggers's value when deleting
    SpinLock::Lock lock(m_lock);
    auto it = m_loggers.find(logger->getName());
    if (it == m_loggers.end())
        return;
    m_loggers.erase(it->first);
}

Logger::ptr LoggerMgr::getLogger(const std::string &name) {
    // Prevent other threads from modifying m_loggers's value when getting a logger
    SpinLock::Lock lock(m_lock);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end())
        return it->second;
    Logger::ptr logger(new Logger(name));
    m_loggers[name] = logger;
    return logger;
}

}  // namespace tigerkin