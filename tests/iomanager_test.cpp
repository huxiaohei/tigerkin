/*****************************************************************
* Description 
* Email huxiaoheigame@gmail.com
* Created on 2021/10/17
* Copyright (c) 2021 虎小黑
****************************************************************/

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "../src/iomamager.h"
#include "../src/macro.h"
#include "../src/scheduler.h"
#include "../src/thread.h"

int sock_a = 0;
int sock_b = 0;

void co_func_a() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func a start";
    sock_a = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock_a, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "14.215.177.38", &addr.sin_addr.s_addr);
    if (!connect(sock_a, (const sockaddr *)&addr, sizeof(addr))) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "connect fail";
    } else if (errno == EINPROGRESS) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "add event";
        tigerkin::IOManager::GetThis()->addEvent(sock_a, tigerkin::IOManager::Event::WRITE, []() {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "write connect a";
            close(sock_a);
        });
    } else {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(TEST)) << "ERRNO:" << strerror(errno);
    }
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func a end";
}

void co_func_b() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func b start";
        sock_b = socket(AF_INET, SOCK_STREAM, 0);
        fcntl(sock_b, F_SETFL, O_NONBLOCK);
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(80);
        inet_pton(AF_INET, "121.14.77.221", &addr.sin_addr.s_addr);
        if (!connect(sock_b, (const sockaddr *)&addr, sizeof(addr))) {
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "connect fail";
        } else if (errno == EINPROGRESS) {
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "add event";
            tigerkin::IOManager::GetThis()->addEvent(sock_b, tigerkin::IOManager::Event::WRITE, []() {
                TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "write connect b";
                close(sock_b);
            });
        } else {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(TEST)) << "ERRNO:" << strerror(errno);
        }
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func b end";
}

void test_simple_test() {
    tigerkin::IOManager iom(1, false, "IOManager");
    std::cout << "test_simple_test" << std::endl;
    iom.schedule(&co_func_a);
    iom.schedule(&co_func_b);
}



int main(int argc, char **argv) {
    std::cout << "iomanager_test start" << std::endl;
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::Thread::SetName("iomanager test");
    test_simple_test();
    std::cout << "iomanager_test end" << std::endl;
    return 0;
}