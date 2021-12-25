/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tigerkin.h"

void run() {
    tigerkin::http::HttpServer::ptr server(new tigerkin::http::HttpServer);
    tigerkin::Address::ptr addr = tigerkin::IpAddress::LookupAnyIpAddress("0.0.0.0:8080");
    while (!server->bind(addr)) {
        sleep(2);
    }
    tigerkin::http::ServletDispatch::ptr dsp = server->getServletDispatch();
    dsp->addServlet("/hello", [](tigerkin::http::HttpRequest::ptr req,
                                 tigerkin::http::HttpResponse::ptr rsp,
                                 tigerkin::http::HttpSession::ptr sess) {
        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "params:";
        const auto params = req->getParams();
        for (auto &it : params) {
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\t" << it.first << ":" << it.second;
        }

        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "cookies:";
        const auto cookies = req->getCookies();
        for (auto &it : cookies) {
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\t" << it.first << ":" << it.second;
        }

        TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "headers:";
        const auto headers = req->getHeaders();
        for (auto &it : headers) {
            TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\t" << it.first << ":" << it.second;
        }

        rsp->setBody("Hello I'm tigerkin!");
        rsp->setHeader("result", "ok");
        rsp->setCookies("cookie", "tigerkin");

        return 0;
    });

    dsp->addServlet("/close", [server](tigerkin::http::HttpRequest::ptr req,
                                       tigerkin::http::HttpResponse::ptr rsp,
                                       tigerkin::http::HttpSession::ptr sess) {
        rsp->setBody("Ok");
        server->stop();
        tigerkin::IOManager::GetThis()->schedule([]() {
            tigerkin::IOManager::GetThis()->stop();
        }, tigerkin::IOManager::GetThis()->getCallerThreadId());
        return 0;
    });
    server->start();
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    tigerkin::IOManager::ptr iom(new tigerkin::IOManager(2));
    iom->schedule(&run);
    iom->start();
    return 0;
}