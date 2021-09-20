/*****************************************************************
* Description 
* Email huxiaoheigame@gmail.com
* Created on 2021/09/19
* Copyright (c) 2021 虎小黑
****************************************************************/

#include <iostream>
#include "../src/log.h"
#include "../src/coroutine.h"

void co_test_funcB() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine B start";
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine B end";
}

void co_test_funcA() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine A start";
    tigerkin::Coroutine::GetThis()->yield();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in coroutine A end";
}


int main(int argc, char **argv) {
    std::cout << "coroutine_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in main function";
    tigerkin::Coroutine::ptr coA(new tigerkin::Coroutine(&co_test_funcA));
    coA->resume();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "in main function";
    coA->resume();
    return 0;
}
