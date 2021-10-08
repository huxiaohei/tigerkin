/*****************************************************************
* Description 
* Email huxiaoheigame@gmail.com
* Created on 2021/09/21
* Copyright (c) 2021 虎小黑
****************************************************************/

#include <iostream>
#include "../src/macro.h"
#include "../src/scheduler.h"
#include "../src/thread.h"
#include "../src/util.h"

void co_func_a() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "COROUTINE FUNCTION A";
}

void test_simple_scheduler() {

    tigerkin::Scheduler::ptr sc(new tigerkin::Scheduler(5, true, "Scheduler"));
    for (int i = 0; i < 10000; ++i) {
        tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_func_a));
        sc->schedule(co, 0);
    }
    time_t now = tigerkin::GetNowMillisecond();
    sc->start();
    sc->stop();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "cost time " << tigerkin::GetNowMillisecond() - now;
}


int main(int argc, char **argv) {
    std::cout << "test tigerkin log start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::Thread::SetName("Main");
    test_simple_scheduler();
    std::cout << "test tigerkin log end" << std::endl;
    return 0;
}


