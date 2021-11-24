/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HTTP_PARSER_H__
#define __TIGERKIN_HTTP_PARSER_H__

#include "http.h"
#include "http_request_parser.h"
#include "http_response_parser.h"

namespace tigerkin {
namespace http {

enum HttpParserError {
    OK = 0,
    INVALID_METHOD_ERROR = -1001,
    INVALID_VERSION_ERROR,
    INVALID_FIELD_ERROR
};

class HttpRequestParser {
   public:
    typedef std::shared_ptr<HttpRequestParser> ptr;

    HttpRequestParser();

    size_t execute(char *data, size_t len);
    int isFinished();
    int hasError();
    HttpRequest::ptr getData() const { return m_data; }
    void setError(HttpParserError v) { m_error = v; }
    uint64_t getContentLength();
    const http_request_parser &getParser() const { return m_parser; }

    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpRequestMaxBodySize();

   private:
    http_request_parser m_parser;
    HttpRequest::ptr m_data;
    HttpParserError m_error;
};

class HttpResponseParser {
   public:
    typedef std::shared_ptr<HttpResponseParser> ptr;

    HttpResponseParser();

    size_t execute(char *data, size_t len, bool chunck);
    int isFinished();
    int hasError();
    HttpResponse::ptr getData() const { return m_data; }
    void setError(HttpParserError v) { m_error = v; }
    uint64_t getContentLength();
    const http_response_parser &getParser() const { return m_parser; }

    static uint64_t GetHttpResponseBufferSize();
    static uint64_t GetHttpResponseMaxBodySize();

   private:
    http_response_parser m_parser;
    HttpResponse::ptr m_data;
    HttpParserError m_error;
};

}  // namespace http
}  // namespace tigerkin

#endif  // !__TIGERKIN_HTTP_PARSER_H__