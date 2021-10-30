/*****************************************************************
* Description 
* Email huxiaoheigame@gmail.com
* Created on 2021/09/21
* Copyright (c) 2021 虎小黑
****************************************************************/

#include "../src/scheduler.h"

#include <iostream>
#include <string>

#include "../src/hook.h"
#include "../src/macro.h"
#include "../src/thread.h"
#include "../src/util.h"

void co_func_a() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "in co A start";
    sleep_f(0.01);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "in co A end";
}

void test_scheduler_use_caller() {
    tigerkin::Scheduler::ptr sc(new tigerkin::Scheduler(3, true, "UserCaller"));
    time_t now = tigerkin::GetNowMillisecond();
    sc->start();
    for (size_t i = 0; i < 10000; ++i) {
        if (i % 2 == 0) {
            tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_func_a));
            sc->schedule(co, 0);
        } else {
            sc->schedule([]() -> void { TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "in function A"; }, 0);
        }
    }
    sc->stop();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "cost time " << tigerkin::GetNowMillisecond() - now;
}

void test_scheduler() {
    tigerkin::Scheduler::ptr sc(new tigerkin::Scheduler(3, false, "NotUseCaller"));
    time_t now = tigerkin::GetNowMillisecond();
    sc->start();
    for (size_t i = 0; i < 10000; ++i) {
        if (i % 2 == 0) {
            tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_func_a));
        } else {
            sc->schedule([]() -> void { TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "in function A"; }, 0);
        }
    }
    sc->stop();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "cost time " << tigerkin::GetNowMillisecond() - now;
}

int main(int argc, char **argv) {
    std::cout << "test tigerkin scheduler start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::Thread::SetName("Main");
    test_scheduler_use_caller();
    test_scheduler();
    std::cout << "test tigerkin scheduler end" << std::endl;
    return 0;
}
