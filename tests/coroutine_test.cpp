/*****************************************************************
* Description 
* Email huxiaoheigame@gmail.com
* Created on 2021/09/19
* Copyright (c) 2021 虎小黑
****************************************************************/

#include "../src/coroutine.h"

#include <iostream>
#include <string>
#include <vector>

#include "../src/macro.h"
#include "../src/thread.h"

void co_test_funcA() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine A start";
    tigerkin::Coroutine::GetThis()->yield();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine A end";
}

void simple_test() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "simple test start";
    tigerkin::Coroutine::ptr coA(new tigerkin::Coroutine(&co_test_funcA));
    coA->resume();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "simple test";
    coA->resume();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "simple end";
}

void co_test_funcB() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine B start";
    sleep(2);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine B end";
}

void thread_test_func() {
    tigerkin::Coroutine::ptr coB(new tigerkin::Coroutine(&co_test_funcB));
    coB->resume();
}

void thread_test() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "thread test start";
    std::vector<tigerkin::Thread::ptr> ths;
    for (int i = 0; i < 3; ++i) {
        ths.push_back(tigerkin::Thread::ptr(new tigerkin::Thread(&thread_test_func, "co_test_" + std::to_string(i))));
    }
    for (size_t i = 0; i < ths.size(); ++i) {
        ths[i]->join();
    }
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "thread test end";
}

void co_test_funcC() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine C start";
    tigerkin::Coroutine::GetThis()->yield();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine C end";
}

void co_test_funcD() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine D start";
    tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_test_funcC));
    co->resume();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine D end";

}

void co_test_funcE() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine E start";
    tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_test_funcD));
    co->resume();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine E end";

}

void muilt_coroutine_test() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "muilt coroutine test start";
    tigerkin::Coroutine::ptr co(new tigerkin::Coroutine(&co_test_funcE));
    co->resume();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "muilt coroutine test";
    tigerkin::Coroutine::Resume(co->getStackId());
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "muilt coroutine test end";
}

int main(int argc, char **argv) {
    tigerkin::Thread::SetName("main");
    std::cout << "coroutine_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    simple_test();
    thread_test();
    muilt_coroutine_test();
    std::cout << "coroutine_test end" << std::endl;
    return 0;
}
