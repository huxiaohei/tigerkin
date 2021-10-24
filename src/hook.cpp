/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include <pthread.h>
#include <unistd.h>

#include "iomamager.h"

namespace tigerkin {

static thread_local bool t_enable_hook = false;

void setEnableHook(bool enable) {
    t_enable_hook = enable;
}

bool isEnableHook() {
    return t_enable_hook;
}

void blockSleep(unsigned int s) {
    sleep(s); 
}

void blockUsleep(unsigned int us) {
    usleep(us);
}

void nonblockSleep(unsigned int s) {
    if (!isEnableHook()) {
        return;
    }
    IOManager *iom = IOManager::GetThis();
    Coroutine::ptr co = Coroutine::GetThis();
    iom->addTimer(
        s * 1000, [co, iom]() {
            iom->schedule(co);
        },
        false);
    Coroutine::Yield();
}

void nonblockUsleep(unsigned int us) {
    if (!isEnableHook()) {
        return;
    }
    IOManager *iom = IOManager::GetThis();
    Coroutine::ptr co = Coroutine::GetThis();
    iom->addTimer(
        us / 1000, [co, iom]() {
            iom->schedule(co);
        },
        false);
    Coroutine::Yield();
}

}  // namespace tigerkin
