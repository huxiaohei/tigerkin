/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_parser.h"

#include "../config.h"

namespace tigerkin {
namespace http {

static ConfigVar<uint64_t>::ptr gHttpRequestBufferSize = Config::Lookup("http.request.bufferSize", (uint64_t)(4 * 1024), "http request buffer size");
static ConfigVar<uint64_t>::ptr gHttpRequestMaxBodySize = Config::Lookup("http.request.maxBodySize", (uint64_t)(64 * 1024 * 1024), "http request max body size");
static ConfigVar<uint64_t>::ptr gHttpResponseBufferSize = Config::Lookup("http.response.bufferSize", (uint64_t)(4 * 1024), "http response buffer size");
static ConfigVar<uint64_t>::ptr gHttpResponseMaxBodySize = Config::Lookup("http.response.maxBodySize", (uint64_t)(64 * 1024 * 1024), "http response max body size");

void on_request_method(void *data, const char *at, size_t length) {
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    HttpMethod m = CharsToHttpMethod(at);
    if (m == HttpMethod::INVALID_METHOD) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "INVALID HTTP RESUEST METHOD:\n\t"
                                                     << "method:" << std::string(at, length);
        parser->setError(HttpParserError::INVALID_METHOD_ERROR);
        return;
    }
    parser->getData()->setMethod(m);
}

void on_request_uri(void *data, const char *at, size_t length) {
}

void on_request_fragment(void *data, const char *at, size_t length) {
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    parser->getData()->setFragment(std::string(at, length));
}

void on_request_path(void *data, const char *at, size_t length) {
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    parser->getData()->setPath(std::string(at, length));
}

void on_request_query_string(void *data, const char *at, size_t length) {
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    parser->getData()->setQuery(std::string(at, length));
}

void on_request_http_version(void *data, const char *at, size_t length) {
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    uint8_t v = 0;
    if (strncmp(at, "HTTP/1.1", length) == 0) {
        v = 0x11;
    } else if (strncmp(at, "HTTP/1.0", length) == 0) {
        v = 0x10;
    } else {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "INVALID HTTP REQUEST VERSION:\n\t"
                                                     << "version:" << std::string(at, length);
        parser->setError(HttpParserError::INVALID_VERSION_ERROR);
        return;
    }
    parser->getData()->setVersion(v);
}

void on_request_header_done(void *data, const char *at, size_t length) {
}

void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
    if (flen == 0) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "INVALID HTTP REQUEST FIELD";
        return;
    }
    parser->getData()->setHeader(std::string(field, flen), std::string(value, vlen));
}

HttpRequestParser::HttpRequestParser()
    : m_error(HttpParserError::OK) {
    m_data.reset(new HttpRequest);
    http_request_parser_init(&m_parser);
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query_string;
    m_parser.http_version = on_request_http_version;
    m_parser.header_done = on_request_header_done;
    m_parser.data = this;
}

size_t HttpRequestParser::execute(char *data, size_t len) {
    size_t offset = http_request_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, (len - offset));
    return offset;
}

int HttpRequestParser::isFinished() {
    return http_request_parser_finish(&m_parser);
}

int HttpRequestParser::hasError() {
    return m_error || http_request_parser_has_error(&m_parser);
}

uint64_t HttpRequestParser::getContentLength() {
    return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
    return gHttpRequestBufferSize->getValue();
}

uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
    return gHttpRequestMaxBodySize->getValue();
}

void on_response_reason_phrase(void *data, const char *at, size_t length) {
}

void on_response_status_code(void *data, const char *at, size_t length) {
}

void on_response_chunk_size(void *data, const char *at, size_t length) {
}

void on_response_http_version(void *data, const char *at, size_t length) {
}

void on_response_header_done(void *data, const char *at, size_t length) {
}

void on_response_last_chunk(void *data, const char *at, size_t length) {
}

void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {
}

HttpResponseParser::HttpResponseParser()
    : m_error(HttpParserError::OK) {
    m_data.reset(new HttpResponse);
    http_response_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason_phrase;
    m_parser.status_code = on_response_status_code;
    m_parser.chunk_size = on_response_chunk_size;
    m_parser.http_version = on_response_http_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
    m_parser.data = this;
}

size_t HttpResponseParser::execute(char *data, size_t len, bool chunck) {
    if (chunck) {
        http_response_parser_init(&m_parser);
    }
    size_t offset = http_response_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, (len - offset));
    return offset;
}

int HttpResponseParser::isFinished() {
    return http_response_parser_is_finished(&m_parser);
}

int HttpResponseParser::hasError() {
    return m_error || http_response_parser_has_error(&m_parser);
}

uint64_t HttpResponseParser::getContentLength() {
    return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

uint64_t HttpResponseParser::GetHttpResponseBufferSize() {
    return gHttpResponseBufferSize->getValue();
}

uint64_t HttpResponseParser::GetHttpResponseMaxBodySize() {
    return gHttpResponseMaxBodySize->getValue();
}

}  // namespace http
}  // namespace tigerkin