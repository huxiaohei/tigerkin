/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/27
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_connection.h"

namespace tigerkin {
namespace http {

HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {
}

HttpConnection::~HttpConnection() {
    TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "CONNECTION DESTROY\n:" << m_socket->toString();
}

HttpResponse::ptr HttpConnection::recvResponse() {
    HttpResponseParser::ptr parser(new HttpResponseParser);
    uint64_t bufferSize = HttpResponseParser::GetHttpResponseBufferSize();
    std::shared_ptr<char> buffer(new char[bufferSize + 1], [](char *ptr) {
        delete[] ptr;
    });
    char *data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, bufferSize - offset);
        if (len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';
        size_t nparse = parser->execute(data, len, false);
        if (parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if (offset == (int)bufferSize) {
            close();
            return nullptr;
        }
        if (parser->isFinished()) {
            break;
        }
    } while (true);

    const http_response_parser &rspParser = parser->getParser();
    std::string body;
    if (rspParser.chunked) {
        int len = offset;
        do {
            bool begin = true;
            do {
                if (!begin || len == 0) {
                    int rt = read(data + len, bufferSize - len);
                    if (rt <= 0) {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if (parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if (len == (int)bufferSize) {
                    close();
                    return nullptr;
                }
                begin = false;
            } while (!parser->isFinished());
            if (rspParser.content_len + 2 <= len) {
                body.append(data, rspParser.content_len);
                memmove(data, data + rspParser.content_len + 2, len - rspParser.content_len - 2);
                len -= rspParser.content_len + 2;
            } else {
                body.append(data, len);
                int left = rspParser.content_len - len + 2;
                while (left > 0) {
                    int rt = read(data, left > (int)bufferSize ? (int)bufferSize : left);
                    if (rt < 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        } while (!rspParser.chunks_done);
    } else {
        int64_t length = parser->getContentLength();
        if (length > 0) {
            body.resize(length);
            int len = 0;
            if (length >= offset) {
                memcpy(&body[0], data, offset);
                len = offset;
            } else {
                memcpy(&body[0], data, length);
                len = offset;
            }
            length -= offset;
            if (length > 0) {
                if (readFixLength(&body[len], length) <= 0) {
                    close();
                    return nullptr;
                }
            }
        }
    }
    if (!body.empty()) {
        auto contentEncoding = parser->getData()->getHeader("content-encoding", "");
        if (strcasecmp(contentEncoding.c_str(), "gzip") == 0) {
            // todo
        } else if (strcasecmp(contentEncoding.c_str(), "deflate") == 0) {
            // todo
        }
        parser->getData()->setBody(body);
    }
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req) {
    std::stringstream ss;
    ss << *req;
    std::string data = ss.str();
    return writeFixLength(data.c_str(), data.size());
}

HttpResult::ptr HttpConnection::Get(const std::string &url,
                                    uint64_t timeout,
                                    const std::map<std::string, std::string> &headers,
                                    const std::string &body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::INVALID_URL, "invalid url: " + url);
    }
    return Get(uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::Get(Uri::ptr uri,
                                    uint64_t timeout,
                                    const std::map<std::string, std::string> &headers,
                                    const std::string &body) {
    return Request(HttpMethod::GET, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::Post(const std::string &url,
                                     uint64_t timeout,
                                     const std::map<std::string, std::string> &headers,
                                     const std::string &body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::INVALID_URL, "invalid url: " + url);
    }
    return Request(HttpMethod::POST, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::Post(Uri::ptr uri,
                                     uint64_t timeout,
                                     const std::map<std::string, std::string> &headers,
                                     const std::string &body) {
    return Request(HttpMethod::POST, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::Request(HttpMethod method,
                                        const std::string &url,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::INVALID_URL, "invalid url: " + url);
    }
    return Request(method, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::Request(HttpMethod method,
                                        Uri::ptr uri,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setMethod(method);
    req->setPath(uri->getPath());
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());
    bool hasHost = false;
    for (auto &it : headers) {
        if (strcasecmp(it.first.c_str(), "connection") == 0) {
            if (strcasecmp(it.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);
            }
            continue;
        }
        if (!hasHost && strcasecmp(it.first.c_str(), "host") == 0) {
            hasHost = !it.second.empty();
        }
        req->setHeader(it.first, it.second);
    }
    if (!hasHost) {
        req->setHeader("Host", uri->getHost());
    }
    req->setBody(body);
    return Request(req, uri, timeout);
}

HttpResult::ptr HttpConnection::Request(HttpRequest::ptr req,
                                        Uri::ptr uri,
                                        uint64_t timeout) {
    bool isSsl = uri->getScheme() == "https";
    Address::ptr addr = uri->createAddress();
    if (!addr) {
        std::stringstream ss;
        ss << "Invalid host:"
           << "\nhost: " << uri->getHost()
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::INVALID_HOST, ss.str());
    }
    Socket::ptr sock = isSsl ? SSLSocket::CreateTCPSocket(addr) : Socket::CreateTCPSocket();
    if (!sock) {
        std::stringstream ss;
        ss << "Create socket fail:"
           << "\naddress: " << addr->toString()
           << "\nerrno: " << errno
           << "\nstrerror: " << strerror(errno)
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::CREATE_SOCKET_ERROR, ss.str());
    }
    if (!sock->connect(addr)) {
        std::stringstream ss;
        ss << "Socket connect fail:"
           << "\naddress: " << addr->toString()
           << "\nerrno: " << errno
           << "\nstrerror: " << strerror(errno)
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::CONNECT_FAIL, ss.str());
    }
    sock->setRecvTimeout(timeout);
    HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);
    if (rt == 0) {
        std::stringstream ss;
        ss << "Socket closed by peer:"
           << "\naddress: " << addr->toString()
           << "\nerrno: " << errno
           << "\nstrerror: " << strerror(errno)
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::SEND_CLOSE_BY_PEER, ss.str());
    }
    if (rt < 0) {
        std::stringstream ss;
        ss << "Socket send error:"
           << "\naddress: " << addr->toString()
           << "\nerrno: " << errno
           << "\nstrerror: " << strerror(errno)
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::SEND_SOCKET_ERROR, ss.str());
    }
    HttpResponse::ptr rsp = conn->recvResponse();
    if (!rsp) {
        std::stringstream ss;
        ss << "Socket send error:"
           << "\naddress: " << addr->toString()
           << "\nerrno: " << errno
           << "\nstrerror: " << strerror(errno)
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::TIMEOUT, ss.str());
    }
    return std::make_shared<HttpResult>(rsp, HttpResult::ErrorCode::OK, "ok");
}

HttpConnectionPool::ptr HttpConnectionPool::Create(const std::string &uriStr,
                                                   const std::string &vhost,
                                                   uint32_t maxSize,
                                                   uint32_t maxAliveTime,
                                                   uint32_t maxRequest) {
    Uri::ptr uri = Uri::Create(uriStr);
    if (!uri) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "INVALID URI: " << uriStr;
        return nullptr;
    }
    return std::make_shared<HttpConnectionPool>(uri->getHost(), vhost, uri->getScheme() == "https", uri->getPort(), maxSize, maxAliveTime, maxRequest);
}

HttpConnectionPool::HttpConnectionPool(const std::string &host,
                                       const std::string &vhost,
                                       bool isHttps,
                                       uint32_t port,
                                       uint32_t maxSize,
                                       uint32_t maxAliveTime,
                                       uint32_t maxRequest)
    : m_host(host), m_vhost(vhost), m_isHttps(isHttps), m_port(port), m_maxSize(maxSize), m_maxAliveTime(maxAliveTime), m_maxRequest(maxRequest) {
}

HttpConnection::ptr HttpConnectionPool::getConnection() {
    uint64_t now = GetNowMillisecond();
    std::vector<HttpConnection *> invalidConns;
    HttpConnection *ptr = nullptr;
    MutexLock::Lock lock(m_mutex);
    while (!m_conns.empty()) {
        HttpConnection *conn = *m_conns.begin();
        m_conns.pop_front();
        if (!conn->isConnected()) {
            invalidConns.push_back(conn);
            continue;
        }
        if ((conn->m_createTime + m_maxAliveTime) > now) {
            invalidConns.push_back(conn);
            continue;
        }
        ptr = conn;
        break;
    }
    lock.unlock();
    for (auto i : invalidConns) {
        delete i;
    }
    m_total -= invalidConns.size();

    if (!ptr) {
        IpAddress::ptr addr = IpAddress::LookupAnyIpAddress(m_host);
        if (!addr) {
            TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "GET IP ADDRESS FAIL\n\t host: " << m_host;
            return nullptr;
        }
        addr->setPort(m_port);
        Socket::ptr sock = m_isHttps ? SSLSocket::CreateTCPSocket(addr) : Socket::CreateTCPSocket(addr);
        if (!sock) {
            TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "CREATE SOCK FAIL address: " << addr->toString();
            return nullptr;
        }
        if (!sock->connect(addr)) {
            TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "SOCK CONNECT FAIL address: " << addr->toString();
            return nullptr;
        }
        ptr = new HttpConnection(sock);
        ++m_total;
    }
    return HttpConnection::ptr(ptr, std::bind(&HttpConnectionPool::ReleasePtr, std::placeholders::_1, this));
}

HttpResult::ptr HttpConnectionPool::get(const std::string &url,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    return request(HttpMethod::GET, url, timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::get(Uri::ptr uri,
                                        uint64_t timeout,
                                        const std::map<std::string, std::string> &headers,
                                        const std::string &body) {
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?") << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#") << uri->getFragment();
    return get(ss.str(), timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::post(const std::string &url,
                                         uint64_t timeout,
                                         const std::map<std::string, std::string> &headers,
                                         const std::string &body) {
    return request(HttpMethod::POST, url, timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::post(Uri::ptr uri,
                                         uint64_t timeout,
                                         const std::map<std::string, std::string> &headers,
                                         const std::string &body) {
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?") << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#") << uri->getFragment();
    return post(ss.str(), timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::request(HttpMethod method,
                                            const std::string &url,
                                            uint64_t timeout,
                                            const std::map<std::string, std::string> &headers,
                                            const std::string &body) {
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setPath(url);
    req->setMethod(method);
    req->setClose(false);
    bool hasHost = false;
    for (auto &it : headers) {
        if (strcasecmp(it.first.c_str(), "connection") == 0) {
            if (strcasecmp(it.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);
            }
            continue;
        }
        if (!hasHost && strcasecmp(it.first.c_str(), "host") == 0) {
            hasHost = !it.second.empty();
        }
        req->setHeader(it.first, it.second);
    }
    if (!hasHost) {
        if (m_vhost.empty()) {
            req->setHeader("Host", m_host);
        } else {
            req->setHeader("Host", m_vhost);
        }
    }
    req->setBody(body);
    return request(req, timeout);
}

HttpResult::ptr HttpConnectionPool::request(HttpMethod method,
                                            Uri::ptr uri,
                                            uint64_t timeout,
                                            const std::map<std::string, std::string> &headers,
                                            const std::string &body) {
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?") << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#") << uri->getFragment();
    return request(method, ss.str(), timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::request(HttpRequest::ptr req, uint64_t timeout) {
    HttpConnection::ptr conn = getConnection();
    if (!conn) {
        std::stringstream ss;
        ss << "Get connection from pool fail:\n"
           << req->toString()
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::POOL_GET_CONNECTION_FAIL, ss.str());
    }
    Socket::ptr sock = conn->getSocket();
    if (!sock) {
        std::stringstream ss;
        ss << "Connection invalid:"
           << "\nhost:" << m_host
           << "\nport:" << m_port
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::POOL_INVALID_CONNECTION, ss.str());
    }
    sock->setRecvTimeout(timeout);
    int rt = conn->sendRequest(req);
    if (rt == 0) {
        std::stringstream ss;
        ss << "Socket closed by peer:"
           << "\nremoteAddress: " << sock->getRemoteAddress()->toString()
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::SEND_CLOSE_BY_PEER, ss.str());
    }
    if (rt < 0) {
        std::stringstream ss;
        ss << "Socket send error:"
           << "\nremoteAddress: " << sock->getRemoteAddress()->toString()
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::SEND_SOCKET_ERROR, ss.str());
    }
    HttpResponse::ptr rsp = conn->recvResponse();
    if (!rsp) {
        std::stringstream ss;
        ss << "Socket send error:"
           << "\nremoteAddress: " << sock->getRemoteAddress()->toString()
           << "\ntimeout: " << timeout;
        return std::make_shared<HttpResult>(nullptr, HttpResult::ErrorCode::TIMEOUT, ss.str());
    }
    return std::make_shared<HttpResult>(rsp, HttpResult::ErrorCode::OK, "ok");
}

void HttpConnectionPool::ReleasePtr(HttpConnection *ptr, HttpConnectionPool *pool) {
    ++ptr->m_request;
    if (!ptr->isConnected() ||
        (ptr->m_createTime + pool->m_maxAliveTime) >= (uint64_t)GetNowMillisecond() ||
        ptr->m_request >= pool->m_maxRequest) {
        delete ptr;
        --pool->m_total;
        return;
    }
    MutexLock::Lock lock(pool->m_mutex);
    pool->m_conns.push_back(ptr);
}

}  // namespace http
}  // namespace tigerkin