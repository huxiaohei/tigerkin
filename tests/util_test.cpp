/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/util.h"

#include "../src/log.h"

void test_backtrace() {
    TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << tigerkin::BacktraceToString(10, 0, "\t");
}

int main() {
    std::cout << "util_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    test_backtrace();
    std::cout << "util_test end" << std::endl;
}