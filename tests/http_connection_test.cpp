/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/28
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tigerkin.h"

void test_http_result() {
    auto rsp = std::make_shared<tigerkin::http::HttpResponse>();
    rsp->setClose(true);
    rsp->setBody("Hello! I'm tigerkin!");
    rsp->setHeader("Host", "http://www.huxiaohei.com");
    auto ret = std::make_shared<tigerkin::http::HttpResult>(rsp, tigerkin::http::HttpResult::ErrorCode::OK, "ok");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << ret->toString();
}

void test_http_connection() {
    auto iom = std::make_shared<tigerkin::IOManager>(2);
    iom->schedule([]() {
        auto rst = tigerkin::http::HttpConnection::Get("http://www.huxiaohei.com", 5000);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << rst->toString();
    });
    iom->schedule([iom]() { iom->stop(); }, tigerkin::GetThreadId());
    iom->start();
}

void test_http_connection_pool() {
    auto iom = std::make_shared<tigerkin::IOManager>(2);
    auto connPool = tigerkin::http::HttpConnectionPool::Create("http://www.huxiaohei.com", "", 20, 30000, 5);
    static std::atomic<int> i{0};
    iom->addTimer(1000, [iom, connPool]() {
        ++i;
        if (i == 20) {
            iom->schedule([iom]() {
                iom->stop();
            }, iom->getCallerThreadId());
        }
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "GET START " << i;
        auto rst = connPool->get("/ChapterOne/", 5000);
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "GET END";
    }, true);
    iom->start();
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "HTTP CONNECTION TEST START";
    // test_http_result();
    // test_http_connection();
    test_http_connection_pool();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "HTTP CONNECTION TEST END";
    return 0;
}
