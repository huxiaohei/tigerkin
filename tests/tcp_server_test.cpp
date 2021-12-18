/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/05
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tcp_server.h"

#include "../src/address.h"
#include "../src/hook.h"
#include "../src/iomanager.h"

void run() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "RUN START";
    tigerkin::Address::ptr addr1 = tigerkin::Address::LookupAny("0.0.0.0:8035");
    std::string path = "/home/liuhu/tigerkin/bin/unix";
    tigerkin::UnixAddress::ptr addr2 = tigerkin::UnixAddress::ptr(new tigerkin::UnixAddress(path));

    std::vector<tigerkin::Address::ptr> addrs;
    addrs.push_back(addr1);
    addrs.push_back(addr2);
    std::vector<tigerkin::Address::ptr> errAddrs;
    tigerkin::TcpServer::ptr tcpServer = tigerkin::TcpServer::ptr(new tigerkin::TcpServer);
    while (!tcpServer->bind(addrs, errAddrs)) {
        for (int i = addrs.size(); i >= 0; --i) {
            bool del = false;
            for (auto addr : errAddrs) {
                if (addr == addrs.at(i)) {
                    del = true;
                    break;
                }
            }
            if (del) {
                addrs.erase(addrs.begin() + i);
            }
        }
        errAddrs.clear();
        sleep(2);
    }
    tcpServer->start();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "RUN END";
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "tcp_server_test start";
    tigerkin::IOManager iom(2, true, "TCP Server");
    iom.schedule(&run);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "tcp_server_test end";
    sleep_f(1000);
    return 0;
}