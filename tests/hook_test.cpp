/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/hook.h"

#include "../src/iomamager.h"
#include "../src/macro.h"

void test_block_sleep() {
    tigerkin::IOManager iom(1, true, "test_block_sleep");
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 2 second start";
        tigerkin::blockSleep(2);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 2 second end";
    });
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 3 second start";
        tigerkin::blockSleep(3);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 3 second end";
    });
}

void test_nonblock_sleep() {
    tigerkin::IOManager iom(1, false, "test_nonblock_sleep");
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 2 second start";
        tigerkin::nonblockSleep(2);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 2 second end";
    });
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 3 second start";
        tigerkin::nonblockSleep(3);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 3 second end";
    });
    tigerkin::blockSleep(6);
}

int main(int argc, char **argv) {
    std::cout << "hook_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    test_block_sleep();
    test_nonblock_sleep();
    std::cout << "hook_test end" << std::endl;
}
