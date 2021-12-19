/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tigerkin.h"

class EchoServer : public tigerkin::TcpServer {
   public:
    EchoServer(int type);
    void handleClient(tigerkin::Socket::ptr client) override;

   private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    : m_type(type) {
}

void EchoServer::handleClient(tigerkin::Socket::ptr client) {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(EXAMPLE)) << "HANDLE CLIENT:"
                                                  << client->toString();
    tigerkin::ByteArray::ptr ba(new tigerkin::ByteArray(1024));
    while (true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);
        int rt = client->recv(&iovs[0], iovs.size());
        if (rt == 0) {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(EXAMPLE)) << "CLIENT CLOSE:"
                                                          << client->toString();
            break;
        } else if (rt < 0) {
            TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(EXAMPLE)) << "CLIENT ERROR:"
                                                          << client->toString()
                                                          << "\n\terrno:" << errno
                                                          << "\n\tstrerror:" << strerror(errno);
            break;
        }
        if (m_type == 1) {
            ba->setOffset(ba->getOffset() + rt);
            ba->setOffset(ba->getOffset() - rt);
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(EXAMPLE)) << "RECEIVE:\n\t"
                                                           << "rt:" << rt << "\n\t"
                                                           << "msg:" << ba->toString();
        } else {
            ba->setOffset(ba->getOffset() + rt);
            ba->setOffset(ba->getOffset() - rt);
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(EXAMPLE)) << "RECEIVE:\n\t"
                                                           << "rt:" << rt << "\n\t"
                                                           << "msg:" << ba->toHexString();
        }
        ba->setOffset(ba->getOffset() + rt);
    }
}

void run(int type) {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(EXAMPLE)) << "ECHO SERVER START";
    tigerkin::Address::ptr addr = tigerkin::Address::LookupAny("0.0.0.0:8080");
    EchoServer::ptr echoSvr(new EchoServer(type));
    while (!echoSvr->bind(addr)) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(EXAMPLE)) << "BIND ERROR";
        sleep(2);
    }
    echoSvr->start();
    TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(EXAMPLE)) << "ECHO SERVER END";
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/examples/etc/log.yml", "logs");
    tigerkin::IOManager::ptr iom(new tigerkin::IOManager(2));
    iom->schedule(std::bind(&run, 1));
    iom->start();
}