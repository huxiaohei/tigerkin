/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/tigerkin.h"

int32_t pingpong(tigerkin::http::HttpRequest::ptr req,
                 tigerkin::http::HttpResponse::ptr rsp,
                 tigerkin::http::HttpSession::ptr sess) {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(EXAMPLE)) << "receive http:\n"
                                                   << req->toString();
    if (req->getMethod() == tigerkin::http::HttpMethod::GET) {
        rsp->setBody(req->toString());
    } else {
        rsp->setBody(req->getBody());
    }
    rsp->setHeaders(req->getHeaders());
    return 0;
}

int32_t hello(tigerkin::http::HttpRequest::ptr req,
              tigerkin::http::HttpResponse::ptr rsp,
              tigerkin::http::HttpSession::ptr sess) {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(EXAMPLE)) << "receive http:\n"
                                                   << req->toString();
    rsp->setBody("Hello! I'm tigerkin!");
    return 0;
}

void run() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(EXAMPLE)) << "HTTP SERVER START";
    auto httpServer = std::make_shared<tigerkin::http::HttpServer>();
    auto addr = tigerkin::IpAddress::LookupAnyIpAddress("0.0.0.0:8080");
    while (!httpServer->bind(addr)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(EXAMPLE)) << "HTTP SERVER BIND ERROR\n"
                                                       << addr->toString();
        sleep(2000);
    }
    auto dispatch = httpServer->getServletDispatch();
    dispatch->addServlet("/", &hello);
    dispatch->addServlet("/hello", &hello);
    dispatch->addServlet("/pingpong", &pingpong);
    dispatch->addServlet("/stop", [httpServer](tigerkin::http::HttpRequest::ptr req,
                                                    tigerkin::http::HttpResponse::ptr rsp,
                                                    tigerkin::http::HttpSession::ptr sess) {
        auto iom = tigerkin::IOManager::GetThis();
        iom->addTimer(2000, []() {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(EXAMPLE)) << "HTTP SERVER STOP";
        });
        rsp->setBody("close http server");
        return 0;
    });
    httpServer->start();
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("../conf/log.yml", "logs");
    tigerkin::IOManager::ptr iom(new tigerkin::IOManager(2));
    iom->schedule(&run);
    iom->start();
    return 0;
}