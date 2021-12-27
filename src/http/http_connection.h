/*****************************************************************
 * Description 作为http客户端的链接实例
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/27
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HTTP_CONNECTION_H__
#define __TIGERKIN_HTTP_CONNECTION_H__

#include <atomic>
#include <list>

#include "../stream/socket_stream.h"
#include "../thread.h"
#include "../uri.h"
#include "http.h"
#include "http_parser.h"

namespace tigerkin {
namespace http {

struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;

    typedef enum {
        OK = 0,
        INVALID_URL = 1,
        INVALID_HOST = 2,
        CONNECT_FAIL = 3,
        SEND_CLOSE_BY_PEER = 4,
        SEND_SOCKET_ERROR = 5,
        TIMEOUT = 6,
        CREATE_SOCKET_ERROR = 7,
        POOL_GET_CONNECTION_FAIL = 8,
        POOL_INVALID_CONNECTION = 9
    } ErrorCode;

    HttpResult(HttpResponse::ptr _rsp, ErrorCode _errCode = ErrorCode::OK, const std::string &_errDes = "")
        : rsp(_rsp), errCode(_errCode), errDes(_errDes) {}

    std::string toString() const {
        std::stringstream ss;
        ss << "\nerrCode:" << errCode
           << "\nerrDes:" << errDes
           << "\n"
           << rsp->toString();
        return ss.str();
    }

    HttpResponse::ptr rsp;
    ErrorCode errCode;
    std::string errDes;
};

class HttpConnectionPool;

class HttpConnection : public SocketStream {
    friend class HttpConnectionPool;

   public:
    typedef std::shared_ptr<HttpConnection> ptr;

    HttpConnection(Socket::ptr sock, bool owner = true);
    ~HttpConnection();

    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr req);

   public:
    static HttpResult::ptr Get(const std::string &url,
                               uint64_t timeout,
                               const std::map<std::string, std::string> &headers = {},
                               const std::string &body = "");

    static HttpResult::ptr Get(Uri::ptr uri,
                               uint64_t timeout,
                               const std::map<std::string, std::string> &headers = {},
                               const std::string &body = "");

    static HttpResult::ptr Post(const std::string &url,
                                uint64_t timeout,
                                const std::map<std::string, std::string> &headers = {},
                                const std::string &body = "");

    static HttpResult::ptr Post(Uri::ptr uri,
                                uint64_t timeout,
                                const std::map<std::string, std::string> &headers = {},
                                const std::string &body = "");

    static HttpResult::ptr Request(HttpMethod method,
                                   const std::string &url,
                                   uint64_t timeout,
                                   const std::map<std::string, std::string> &headers = {},
                                   const std::string &body = "");

    static HttpResult::ptr Request(HttpMethod method,
                                   Uri::ptr uri,
                                   uint64_t timeout,
                                   const std::map<std::string, std::string> &headers = {},
                                   const std::string &body = "");

    static HttpResult::ptr Request(HttpRequest::ptr req,
                                   Uri::ptr uri,
                                   uint64_t timeout);

   private:
    uint64_t m_createTime = 0;
    uint64_t m_request = 0;
};

class HttpConnectionPool {
   public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;

    static HttpConnectionPool::ptr Create(const std::string &uriStr,
                                          const std::string &vhost,
                                          uint32_t maxSize,
                                          uint32_t maxAliveTime,
                                          uint32_t maxRequest);

    HttpConnectionPool(const std::string &host,
                       const std::string &vhost,
                       bool isHttps,
                       uint32_t port,
                       uint32_t maxSize,
                       uint32_t maxAliveTime,
                       uint32_t maxRequest);

    HttpConnection::ptr getConnection();

    HttpResponse::ptr get(const std::string &url,
                          uint64_t timeout,
                          const std::map<std::string, std::string> &headers = {},
                          const std::string &body = "");

    HttpResponse::ptr get(Uri::ptr uri,
                          uint64_t timeout,
                          const std::map<std::string, std::string> &headers = {},
                          const std::string &body = "");

    HttpResponse::ptr post(const std::string &url,
                           uint64_t timeout,
                           const std::map<std::string, std::string> &headers = {},
                           const std::string &body = "");

    HttpResponse::ptr post(Uri::ptr uri,
                           uint64_t timeout,
                           const std::map<std::string, std::string> &headers = {},
                           const std::string &body = "");

    HttpResponse::ptr request(HttpMethod method,
                              const std::string &url,
                              uint64_t timeout,
                              const std::map<std::string, std::string> &headers = {},
                              const std::string &body = "");

    HttpResponse::ptr request(HttpMethod method,
                              Uri::ptr uri,
                              uint64_t timeout,
                              const std::map<std::string, std::string> &headers = {},
                              const std::string &body = "");

    HttpResponse::ptr request(HttpRequest::ptr req, uint64_t timeout);

   private:
    static void ReleasePtr(HttpConnection *ptr, HttpConnectionPool *pool);

   private:
    std::string m_host;
    std::string m_vhost;
    bool m_isHttps;
    uint32_t m_port;
    uint32_t m_maxSize;
    uint32_t m_maxAliveTime;
    uint32_t m_maxRequest;

    MutexLock m_mutex;
    std::list<HttpConnection *> m_conns;
    std::atomic<int32_t> m_total = {0};
};

}  // namespace http
}  // namespace tigerkin

#endif