/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/scheduler.h"

#include <iostream>
#include <string>
#include <functional>

#include "../src/hook.h"
#include "../src/macro.h"
#include "../src/thread.h"
#include "../src/util.h"

void co_func_a() {
    // TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in func a start";
    sleep_f(0.01);
    // TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in func a end";
}

void stop_scheduler(tigerkin::Scheduler::ptr sc) {
    sc->stop();
}

void test_scheduler_use_caller() {
    tigerkin::Scheduler::ptr sc(new tigerkin::Scheduler(3, true, "UserCaller"));
    time_t now = tigerkin::GetNowMillisecond();
    for (size_t i = 0; i < 10000; ++i) {
        if (i % 2 == 0) {
            tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_func_a));
            sc->schedule(co);
        } else {
            sc->schedule([]() -> void {
                TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in callback";
            });
        }
    }
    sc->schedule(std::bind(&stop_scheduler, sc), tigerkin::GetThreadId());
    sc->start();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "use caller cost time " << tigerkin::GetNowMillisecond() - now;
}

void test_scheduler() {
    tigerkin::Scheduler::ptr sc(new tigerkin::Scheduler(2, false, "NotUseCaller"));
    time_t now = tigerkin::GetNowMillisecond();
    sc->start();
    for (size_t i = 0; i < 10000; ++i) {
        if (i % 2 == 0) {
            tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_func_a));
            sc->schedule(co);
        } else {
            sc->schedule([]() -> void {
                TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in callback";
            });
        }
    }
    sc->stop();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "not use cost time " << tigerkin::GetNowMillisecond() - now;
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::Thread::SetName("Scheduler Test");
    test_scheduler_use_caller();
    test_scheduler();
    return 0;
}
