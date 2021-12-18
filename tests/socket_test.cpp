/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/10
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/socket.h"

#include "../src/hook.h"
#include "../src/iomanager.h"
#include "../src/macro.h"

void test_socket_send() {
    tigerkin::IpAddress::ptr address = tigerkin::IpAddress::LookupAnyIpAddress("www.baidu.com:80");
    tigerkin::Socket::ptr sock = tigerkin::Socket::CreateTCPSocket();
    if (!sock->connect(address)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(TEST)) << "socket connect error\n\t"
                                                    << "errno:" << errno << "\n\t"
                                                    << "strerror:" << strerror(errno);
        return;
    }
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    ssize_t rt = sock->send(buff, sizeof(buff));
    if (rt <= 0) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(TEST)) << "socket send error\n\t"
                                                    << "errno:" << errno << "\n\t"
                                                    << "strerror:" << strerror(errno) << "\n\t"
                                                    << "rt:" << rt;
        return;
    }
    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());
    if (rt <= 0) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(TEST)) << "socket recv error\n\t"
                                                    << "errno:" << errno << "\n\t"
                                                    << "strerror:" << strerror(errno) << "\n\t"
                                                    << "rt:" << rt;
        return;
    }
    buffs.resize(rt);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "socket recv:\n\t"
                                                  << buffs;
}

int main() {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "socket_test start";
    tigerkin::IOManager::ptr iom(new tigerkin::IOManager(1, false));
    iom->start();
    iom->schedule(&test_socket_send);
    iom->stop();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "socket_test end";
    return 0;
}