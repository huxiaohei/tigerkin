/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/22
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http.h"

#include "../util.h"

namespace tigerkin {
namespace http {

HttpMethod StringToHttpMethod(const std::string &m) {
#define XX(num, name, string)              \
    if (strcmp(#string, m.c_str()) == 0) { \
        return HttpMethod::name;           \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

HttpMethod CharsToHttpMethod(const char *m) {
#define XX(num, name, string)                        \
    if (strncmp(#string, m, strlen(#string)) == 0) { \
        return HttpMethod::name;                     \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

const char *HttpMethodToString(const HttpMethod &m) {
    static const char *s_method_string[] = {
#define XX(num, name, string) #string,
        HTTP_METHOD_MAP(XX)
#undef XX
    };
    uint32_t idx = (uint32_t)m;
    if (idx > (sizeof(s_method_string) / sizeof(s_method_string[0]))) {
        return "UNKNOWN";
    }
    return s_method_string[idx];
}

const char *HttpStatusToString(const HttpStatus &s) {
    switch (s) {
#define XX(code, name, msg) \
    case HttpStatus::name:  \
        return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
            return "UNKNOWN";
    }
}

bool CaseInsensitiveLess::operator()(const std::string &lhs, const std::string &rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpResponse::HttpResponse(uint8_t version, bool close)
    : m_status(HttpStatus::OK), m_version(version), m_close(close), m_websocket(false) {
}

void HttpResponse::delHeader(const std::string &key) {
    m_headers.erase(key);
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
    m_headers[key] = value;
}

void HttpResponse::setRedirect(const std::string &uri) {
    m_status = HttpStatus::FOUND;
    setHeader("Location", uri);
}

std::string HttpResponse::getHeader(const std::string &key, const std::string &def) {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}

void HttpResponse::setCookies(const std::string &key, const std::string &val, time_t expired, const std::string &path, const std::string &domain, bool secure) {
    std::stringstream ss;
    ss << key << "=" << val;
    if (expired > 0) {
        ss << ";expires=" << Time2Str(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
    }
    if (!domain.empty()) {
        ss << ";dlomain=" << domain;
    }
    if (!path.empty()) {
        ss << ";path=" << path;
    }
    if (secure) {
        ss << ";secure";
    }
    m_cookies.push_back(ss.str());
}

std::string HttpResponse::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream &HttpResponse::dump(std::ostream &os) const {
    os << "HTTP/"
       << ((uint32_t)(m_version) >> 4) << "." << ((uint32_t)(m_version & 0x0F))
       << " " << (uint32_t)m_status
       << " " << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason)
       << "\r\n";

    for (auto &it : m_headers) {
        if (!m_websocket && strcasecmp(it.first.c_str(), "connection") == 0) {
            continue;
        }
        os << it.first << ":" << it.second << "\r\n";
    }
    for (auto &it : m_cookies) {
        os << "Set-Cookie: " << it << "\r\n";
    }
    if (!m_websocket) {
        os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    : m_method(HttpMethod::GET), m_version(version), m_close(close), m_websocket(false), m_parserParamFlag(0), m_path("/") {
}

void HttpRequest::init() {
    std::string conn = getHeader("connection");
    if (!conn.empty()) {
        if (strcasecmp(conn.c_str(), "keep-alive") == 0) {
            m_close = false;
        } else {
            m_close = true;
        }
    }
}

void HttpRequest::initParam() {
    initQueryParam();
    initBodyParam();
    initCookies();
}

void HttpRequest::initQueryParam() {
    if (m_parserParamFlag & 0x1) {
        return;
    }
}

void HttpRequest::initBodyParam() {
}

void HttpRequest::initCookies() {
}

std::string HttpRequest::getHeader(const std::string &key, const std::string &def) const {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}

void HttpRequest::delHeader(const std::string &key) {
    m_headers.erase(key);
}

void HttpRequest::setHeader(const std::string &key, const std::string &val) {
    m_headers[key] = val;
}

std::string HttpRequest::getParam(const std::string &key, const std::string &def) {
    auto it = m_params.find(key);
    return it == m_params.end() ? def : it->second;
}

void HttpRequest::delParam(const std::string &key) {
    m_params.erase(key);
}

void HttpRequest::setParam(const std::string &key, const std::string &val) {
    m_params[key] = val;
}

std::string HttpRequest::getCookie(const std::string &key, const std::string &def) {
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? def : it->second;
}

void HttpRequest::delCookie(const std::string &key) {
    m_cookies.erase(key);
}

void HttpRequest::setCookie(const std::string &key, const std::string &val) {
    m_cookies[key] = val;
}

std::ostream &HttpRequest::dump(std::ostream &os) const {
    os << HttpMethodToString(m_method) << " "
       << m_path
       << (m_query.empty() ? "" : "?") << m_query
       << (m_fragment.empty() ? "" : "#") << m_fragment
       << " HTTP/" << ((uint32_t)(m_version >> 4)) << "." << ((uint32_t)(m_version & 0x0F)) << "\r\n";
    if (!m_websocket) {
        os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }
    for (auto &it : m_headers) {
        if (!m_websocket && strcasecmp(it.first.c_str(), "connection") == 0) {
            continue;
        }
        os << it.first << ":" << it.second << "\r\n";
    }
    if (m_cookies.size() > 0) {
        os << "Set-Cookie:"
           << "\n";
        for (auto &it : m_cookies) {
            os << "\t" << it.first << ":" << it.second << "\n";
        }
    }
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

std::string HttpRequest::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &request) {
    return request.dump(os);
}

std::ostream &operator<<(std::ostream &os, const HttpResponse &response) {
    return response.dump(os);
}

}  // namespace http
}  // namespace tigerkin