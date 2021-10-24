/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "hook.h"

#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>

#include "iomamager.h"

namespace tigerkin {

static thread_local bool t_enable_hook = false;

#define HOOK_FUNC(XX) \
    XX(sleep);        \
    XX(usleep)

void hookInit() {
    static bool isInit = false;
    if (isInit) {
        return;
    }
#define XX(name) name##_f = (name##_func)dlsym(RTLD_NEXT, #name);
    HOOK_FUNC(XX);
#undef XX
}

struct _HookIniter {
    _HookIniter() {
        hookInit();
    }
};
static _HookIniter s_hookIniter;

void setEnableHook(bool enable) {
    t_enable_hook = enable;
}

bool isEnableHook() {
    return t_enable_hook;
}

}  // namespace tigerkin

extern "C" {
#define XX(name) name##_func name##_f = nullptr;
HOOK_FUNC(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!tigerkin::isEnableHook()) {
        return sleep_f(seconds);
    }
    tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
    tigerkin::Coroutine::ptr co = tigerkin::Coroutine::GetThis();
    iom->addTimer(
        seconds * 1000, [co, iom]() {
            iom->schedule(co);
        },
        false);
    tigerkin::Coroutine::Yield();
    return 0;
}

int usleep(useconds_t usec) {
    if (!tigerkin::isEnableHook()) {
        return usleep(usec);
    }
    tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
    tigerkin::Coroutine::ptr co = tigerkin::Coroutine::GetThis();
    iom->addTimer(
        usec / 1000, [co, iom]() {
            iom->schedule(co);
        },
        false);
    tigerkin::Coroutine::Yield();
    return 0;
}
}
