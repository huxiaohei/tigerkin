/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/01
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "util.h"

#include <execinfo.h>
#include <time.h>

#include "coroutine.h"
#include "macro.h"

namespace tigerkin {

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint64_t GetCoroutineId() {
    return Coroutine::GetCoId();
}

void Backtrace(std::vector<std::string> &bt, int size, int skip) {
    void **array = (void **)malloc(sizeof(void *) * size);
    size_t s = ::backtrace(array, size);
    char **strings = backtrace_symbols(array, s);
    if (strings == NULL) {
        free(array);
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "backtrace symbols error:";
        return;
    }
    for (size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }
    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string &prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    ss << "\n";
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

std::time_t GetNowMillisecond() {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>
        tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    return tp.time_since_epoch().count();
}

std::time_t GetNowSecond() {
    time_t timestamp;
    return time(&timestamp);
}

std::string Time2Str(time_t ts, const std::string &format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

}  // namespace tigerkin