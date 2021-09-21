/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/thread.h"

#include <vector>

#include "../src/macro.h"

void threadExecutCallback() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "thread begin runing\n"
                                               << "\tname:" << tigerkin::Thread::GetName() << "\n"
                                               << "\tid:" << tigerkin::Thread::GetThis()->getId();
    // 1. simulate the thread cost time
    // 2. let you have time to check the thread info
    sleep(20);
    /*
        use cmd check the thread
            1. find pid `ps -aux | grep threadMutex_`
            2. check the thread info `top -H -p [pid]`
    */
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "thread info:\n"
                                               << "\tname:" << tigerkin::Thread::GetName() << "\n"
                                               << "\tid:" << tigerkin::Thread::GetThis()->getId()
                                               << "\t will end";
}

void testThreadExecut() {
    std::vector<tigerkin::Thread::ptr> thrs;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&threadExecutCallback, "threadExecut_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
}

int main() {
    std::cout << "thread_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    testThreadExecut();
    std::cout << "thread_test end" << std::endl;
    return 0;
}