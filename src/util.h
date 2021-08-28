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

#include <iostream>

namespace tigerkin {

pid_t GetThreadId();
uint32_t GetFiberId();

}  // namespace tigerkin

#endif  // !__TIGERKIN__UTIL_H__