/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/28
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "../src/http/http_parser.h"

const char request_data[] =
    "POST / HTTP/1.1\r\n"
    "Host: www.huxiaohei.com\r\n"
    "Content-Length: 11\r\n\r\n"
    "hello world";

void test_parser_request() {
    tigerkin::http::HttpRequestParser parser;
    std::string tmp = request_data;
    size_t rt = parser.execute(&tmp[0], tmp.size());
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "rt:" << rt << "\n\t"
                                                << "is finished: " << parser.isFinished() << "\n\t"
                                                << "has error: " << parser.hasError() << "\n\t"
                                                << "content-length: " << parser.getContentLength();
    tmp.resize(tmp.size() - rt);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "request:\n"
                                                << parser.getData()->toString();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n"
                                                << tmp;
}

const char response_data[] =
    "HTTP/1.1 405 Not Allowed\r\n"
    "Server: nginx/1.20.1\r\n"
    "Date: Sun, 28 Nov 2021 10:49:35 GMT\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 157\r\n"
    "Connection: close\r\n\r\n"
    "<html>\r\n"
    "<head><title>405 Not Allowed</title></head>\r\n"
    "<body>\r\n"
    "<center><h1>405 Not Allowed</h1></center>\r\n"
    "<hr><center>nginx/1.20.1</center>\r\n"
    "</body>\r\n"
    "</html>";

void test_parser_response() {
    tigerkin::http::HttpResponseParser parser;
    std::string tmp = response_data;
    size_t rt = parser.execute(&tmp[0], tmp.size(), false);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n\t"
                                                << "rt: " << rt << "\n\t"
                                                << "is finished: " << parser.isFinished() << "\n\t"
                                                << "has error: " << parser.hasError() << "\n\t"
                                                << "content-length:" << parser.getContentLength();
    tmp.resize(tmp.size() - rt);
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "request:\n"
                                                << parser.getData()->toString();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "\n"
                                                << tmp;
}

int main(int argc, char **argv) {
    tigerkin::SingletonLoggerMgr::GetInstance()->addLoggers("/home/liuhu/tigerkin/conf/log.yml", "logs");
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "http parser test start";
    test_parser_request();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "-------------------";
    test_parser_response();
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(TEST)) << "http parser test end";
    return 0;
}
