/*****************************************************************
 * Description Hook
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HOOK_H__
#define __TIGERKIN_HOOK_H__

#include <time.h>
#include <unistd.h>

namespace tigerkin {

void setEnableHook(bool enable);
bool isEnableHook();

}  // namespace tigerkin

extern "C" {

typedef unsigned int (*sleep_func)(unsigned int seconds);
extern sleep_func sleep_f;

typedef int (*usleep_func)(useconds_t usec);
extern usleep_func usleep_f;

typedef int (*nanosleep_func)(const struct timespec *rqtp, struct timespec *rmtp);
extern nanosleep_func nanosleep_f;
}

#endif
