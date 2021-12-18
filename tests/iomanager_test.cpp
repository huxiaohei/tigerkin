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

#include "../src/hook.h"
#include "../src/iomanager.h"
#include "../src/macro.h"
#include "../src/scheduler.h"
#include "../src/thread.h"

int sock_a = 0;
int sock_b = 0;

void co_func_a() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func a start";
    sock_a = socket_f(AF_INET, SOCK_STREAM, 0);
    fcntl_f(sock_a, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "14.215.177.38", &addr.sin_addr.s_addr);
    if (!connect_f(sock_a, (const sockaddr *)&addr, sizeof(addr))) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "connect fail";
    } else if (errno == EINPROGRESS) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "add event";
        tigerkin::IOManager::GetThis()->addEvent(sock_a, tigerkin::IOManager::Event::WRITE, []() {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "write connect a";
            close_f(sock_a);
        });
    } else {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(TEST)) << "ERRNO:" << strerror(errno);
    }
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func a end";
}

void co_func_b() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func b start";
    sock_b = socket_f(AF_INET, SOCK_STREAM, 0);
    fcntl_f(sock_b, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "121.14.77.221", &addr.sin_addr.s_addr);
    if (!connect_f(sock_b, (const sockaddr *)&addr, sizeof(addr))) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "connect fail";
    } else if (errno == EINPROGRESS) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "add event";
        tigerkin::IOManager::GetThis()->addEvent(sock_b, tigerkin::IOManager::Event::WRITE, []() {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(TEST)) << "write connect b";
            close_f(sock_b);
        });
    } else {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(TEST)) << "ERRNO:" << strerror(errno);
    }
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "co func b end";
}

void stop_iom(tigerkin::IOManager::ptr iom) {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "IOM STOP";
    iom->stop();
}

void test_iom() {
    tigerkin::IOManager::ptr iom(new tigerkin::IOManager(2, true, "IOManager"));
    iom->schedule(&co_func_a);
    iom->schedule(&co_func_b);
    iom->schedule(std::bind(&stop_iom, iom), tigerkin::GetThreadId());
    iom->start();
}

void test_timer() {
    tigerkin::IOManager iom(2, false, "TimerIOManager");
    iom.start();
    tigerkin::Timer::ptr timerA = iom.addTimer(
        2000, []() {
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "I am in timer a";
        },
        true);
    tigerkin::Timer::ptr timerB = iom.addTimer(
        3000, []() {
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "I am in timer b";
        },
        false);
    timerB->reset(1000);
    sleep_f(5);
    timerA->cancel();
    iom.stop();
}

void test_timer_insert() {
    tigerkin::IOManager iom(2, false, "IOManager");
    iom.start();
    iom.addTimer(2000, []() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "timer a";
    });
    iom.addTimer(1000, []() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "timer b";
    });
    sleep_f(5);
    iom.stop();
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::Thread::SetName("iomanager test");
    test_iom();
    test_timer();
    test_timer_insert();
    return 0;
}