/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/28
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/http/http.h"

void test_request() {
    tigerkin::http::HttpRequest::ptr request(new tigerkin::http::HttpRequest);
    request->setHeader("host", "www.huxiaohei.com");
    request->setMethod(tigerkin::http::HttpMethod::POST);
    request->setBody("hello");
    request->setCookie("md5", "0x00xx000");
    std::stringstream os;
    request->dump(os);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n" << os.str();
}

void test_response() {
    tigerkin::http::HttpResponse::ptr response(new tigerkin::http::HttpResponse);
    response->setHeader("host", "www.huxiaohei.com");
    response->setBody("bye-bye");
    response->setCookies("md5", "0x00xx000");
    std::stringstream os;
    response->dump(os);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n" << os.str();
}

int main() {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "http test start";
    test_request();
    test_response();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "http test end";
}
