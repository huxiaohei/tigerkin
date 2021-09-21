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


void co_func_A() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "COROUTINE FUNCTION A";
}

int main(int argc, char **argv) {
    std::cout << "test tigerkin log start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::Thread::SetName("Main");
    tigerkin::Scheduler::ptr sc(new tigerkin::Scheduler());
    tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_func_A));
    sc->schedule(co, 0);
    sc->start();
    sc->stop();
    std::cout << "test tigerkin log end" << std::endl;
    return 0;
}


