/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include <vector>
#include "../src/thread.h"
#include "../src/log.h"

void threadExecutCallback() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "thread begin runing\n"
                                           << "\tid:" << tigerkin::Thread::GetThis()->getId(); 
    // 1. simulate the thread cost time
    // 2. let you have time to check the thread info
    sleep(20);
    /*
        use cmd check the thread
            1. find pid `ps -aux | grep threadMutex_`
            2. check the thread info `top -H -p [pid]`
    */
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "thread info:\n" 
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

int cnt = 0;
tigerkin::ReadWriteMutex s_rwMutex;

void testWriteMutex() {
    // Write locks will cause threads to monopolize resources
    tigerkin::ReadWriteMutex::WriteMutex lock(s_rwMutex);
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

void testReadMutex() {
    // Read lock threads share resources
    tigerkin::ReadWriteMutex::ReadMutex lock(s_rwMutex);
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

tigerkin::Mutex s_mutex;

void testMutex() {
    tigerkin::Mutex::Lock lock(s_mutex);
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

void testLock() {
    std::vector<tigerkin::Thread::ptr> thrs;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testWriteMutex, "writeMutex_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "write mutex cnt = " << cnt;
    
    thrs.clear();
    cnt = 0;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testReadMutex, "readMutex_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "read mutex cnt = " << cnt;

    thrs.clear();
    cnt = 0;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testMutex, "mutex_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "mutex cnt = " << cnt;

}

int main() {
    std::cout << "thread_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    testThreadExecut();
    testLock();
    std::cout << "thread_test end" << std::endl;
    return 0;

}