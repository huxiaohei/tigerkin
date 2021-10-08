/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/01
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN__UTIL_H__
#define __TIGERKIN__UTIL_H__

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <vector>

namespace tigerkin {

pid_t GetThreadId();
uint64_t GetCoroutineId();

void Backtrace(std::vector<std::string> &bt, int size, int skip = 1);
std::string BacktraceToString(int size, int skip = 2, const std::string &prefix = "\t");

std::time_t GetNowMillisecond();
std::time_t GetNowSecond();

}  // namespace tigerkin

#endif  // !__TIGERKIN__UTIL_H__