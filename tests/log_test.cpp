/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/07/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/log.h"

#include <iostream>

#include "../src/thread.h"
#include "../src/util.h"

void testDefaultLog() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_ROOT()) << "default log";
    TIGERKIN_LOG_FMT_INFO(TIGERKIN_LOG_ROOT(), "fmt default %s", "log");
}

void testSimpleLog() {
    tigerkin::Logger::ptr logger(new tigerkin::Logger("testStdSimpleLog"));
    tigerkin::LogAppender::ptr stdAppender(new tigerkin::StdOutLogAppend());
    tigerkin::LogFormatter::ptr formater(new tigerkin::LogFormatter("%d{%Y-%m-%d %H:%M:%S} %t %N %F [%p] [%c] %f:%l %m%n"));
    stdAppender->setFormate(formater);
    logger->addAppender(stdAppender);

    tigerkin::LogAppender::ptr fileAppender(new tigerkin::FileLogAppend("./testFileSimpleLog.txt"));
    fileAppender->setFormate(formater);
    logger->addAppender(fileAppender);

    TIGERKIN_LOG_DEBUG(logger) << "Hello World";
    TIGERKIN_LOG_INFO(logger) << "Hello World";
    TIGERKIN_LOG_WARN(logger) << "Hello World";
    TIGERKIN_LOG_ERROR(logger) << "Hello World";
    TIGERKIN_LOG_FATAL(logger) << "Hello World";

    TIGERKIN_LOG_FMT_DEBUG(logger, "fmt Hello %s", "World");
    TIGERKIN_LOG_FMT_INFO(logger, "fmt Hello %s", "World");
    TIGERKIN_LOG_FMT_WARN(logger, "fmt Hello %s", "World");
    TIGERKIN_LOG_FMT_ERROR(logger, "fmt Hello %s", "World");
    TIGERKIN_LOG_FMT_FATAL(logger, "fmt Hello %s", "World");
}

void testManagerLog() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "I am system logger debug";
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "I am system logger info";
    TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "I am system logger warn";

    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(HALL)) << "I am hall logger debug";
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(HALL)) << "I am hall logger info";
    TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(HALL)) << "I am hall logger warn";

    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(GAME)) << "I am game logger debug";
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(GAME)) << "I am game logger info";
    TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(GAME)) << "I am game logger warn";

    tigerkin::Logger::ptr logger = tigerkin::SingletonLoggerMgr::GetInstance()->getLogger("GAME");
    tigerkin::SingletonLoggerMgr::GetInstance()->deleteLogger(logger);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(GAME)) << "I am game logger debug";
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(GAME)) << "I am game logger info";
    TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(GAME)) << "I am game logger warn";
}

void threadRun1() {
    sleep(1);
    int i = 0;
    while (i < 100) {
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(MUTEX)) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        ++i;
    }
}

void threadRun2() {
    sleep(1);
    int i = 0;
    while (i < 100) {
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(MUTEX)) << "=========================================================";
        ++i;
    }
}

void testThreadSafe() {
    // 可以注释 Mutex 中的实现，让互斥锁失效
    // 在没有互斥锁的情况下日志混乱
    vector<tigerkin::Thread::ptr> thrs;
    for (int i = 0; i < 50; ++i) {
        tigerkin::Thread::ptr th1(new tigerkin::Thread(&threadRun1, "testThreadSafe1_" + std::to_string(i)));
        tigerkin::Thread::ptr th2(new tigerkin::Thread(&threadRun2, "testThreadSafe2_" + std::to_string(i)));
        thrs.push_back(th1);
        thrs.push_back(th2);
    }
    for (size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
}

int main(int argc, char **argv) {
    std::cout << "test tigerkin log start" << std::endl;
    testDefaultLog();
    testSimpleLog();
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    testManagerLog();
    testThreadSafe();
    std::cout << "test tigerkin log end" << std::endl;
    return 0;
}
