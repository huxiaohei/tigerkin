/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/mutex.h"

#include <vector>

#include "../src/log.h"
#include "../src/thread.h"

int cnt = 0;
tigerkin::ReadWriteLock s_rwLock;

void testNoLock() {
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

void testWriteLock() {
    // Write locks will cause threads to monopolize resources
    tigerkin::ReadWriteLock::WriteLock lock(s_rwLock);
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

void testReadLock() {
    // Read lock threads share resources
    tigerkin::ReadWriteLock::ReadLock lock(s_rwLock);
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

tigerkin::MutexLock s_mutexLock;

void testMutex() {
    tigerkin::MutexLock::Lock lock(s_mutexLock);
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

tigerkin::SpinLock s_spinLock;

void testSpin() {
    tigerkin::SpinLock::Lock lock(s_spinLock);
    for (int i = 0; i < 1000; ++i) {
        sleep(0.001);
        cnt += 1;
    }
}

void testLock() {
    std::vector<tigerkin::Thread::ptr> thrs;

    cnt = 0;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testNoLock, "noLock_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "no lock cnt = " << cnt;

    thrs.clear();
    cnt = 0;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testWriteLock, "writeLock_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "write clock cnt = " << cnt;

    thrs.clear();
    cnt = 0;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testReadLock, "readLock_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "read lock cnt = " << cnt;

    thrs.clear();
    cnt = 0;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testMutex, "mutex_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "mutex cnt = " << cnt;

    thrs.clear();
    cnt = 0;
    for (int i = 0; i < 5; ++i) {
        tigerkin::Thread::ptr th(new tigerkin::Thread(&testSpin, "mutex_" + std::to_string(i)));
        thrs.push_back(th);
    }
    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "spin cnt = " << cnt;
}

int main() {
    std::cout << "thread_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    testLock();
    std::cout << "thread_test end" << std::endl;
    return 0;
}