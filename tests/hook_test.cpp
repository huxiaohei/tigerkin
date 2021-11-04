/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/hook.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../src/iomamager.h"
#include "../src/macro.h"

void test_block_sleep() {
    tigerkin::IOManager iom(1, true, "test_block_sleep");
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 2 second start";
        sleep_f(2);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 2 second end";
    });
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 3 second start";
        sleep_f(3);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "block sleep 3 second end";
    });
}

void test_nonblock_sleep() {
    tigerkin::IOManager iom(1, false, "test_nonblock_sleep");
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 2 second start";
        sleep(2);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 2 second end";
    });
    iom.schedule([]() {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 3 second start";
        sleep(3);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "nonblock sleep 3 second end";
    });
    sleep_f(6);
}

void test_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "220.181.38.251", &addr.sin_addr.s_addr);

    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    if (rt) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "socket connect error : " << rt << " errno : " << errno;
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    if (rt <= 0) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "socket send error : " << rt;
        return;
    }
    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    if (rt <= 0) {
        buff.resize(0);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "socket recv error";
        return;
    }
    buff.resize(rt);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "socket recv msg \n" << buff;

}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "hook test start";
    // test_block_sleep();
    // test_nonblock_sleep();
    tigerkin::IOManager iom(3, false, "Hook");
    iom.schedule(&test_socket);
    sleep_f(3);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "hook test end";
}
