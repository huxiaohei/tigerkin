/*****************************************************************
 * Description 
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/util.h"

#include "../src/log.h"
#include "../src/macro.h"

void test_backtrace() {
    // TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << tigerkin::BacktraceToString(10, 0, "\t");
    // TIGERKIN_ASSERT(3 > 5);
    TIGERKIN_ASSERT2(3 > 5, "错误");
    // TIGERKIN_ASSERT2("error2", "ok");
}

int main() {
    std::cout << "util_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    test_backtrace();
    std::cout << "util_test end" << std::endl;
}