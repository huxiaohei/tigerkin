/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/05
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "tcp_server.h"

#include "macro.h"

namespace tigerkin {

static ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = Config::Lookup("tcpServer.readTimeout", (uint64_t)(60 * 1000 * 2), "tcp server read timeout");
static ConfigVar<std::string>::ptr g_tcp_server_name = Config::Lookup("tcpServer.name", (std::string)("tigerkin/1.0.0"), "tcp server name/version");

TcpServer::TcpServer(IOManager *workerIOM, IOManager *acceptIOM)
    : m_workerIOM(workerIOM), m_acceptIOM(acceptIOM), m_rdTimeout(g_tcp_server_read_timeout->getValue()), m_name(g_tcp_server_name->getValue()), m_isStop(true) {
}

TcpServer::~TcpServer() {
    for (auto &it : m_sockets) {
        it->close();
    }
    m_sockets.clear();
}

bool TcpServer::bind(Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> errAddrs;
    addrs.push_back(addr);
    return bind(addrs, errAddrs);
}

bool TcpServer::bind(std::vector<Address::ptr> addrs, std::vector<Address::ptr> &errAddrs) {
    bool rt = true;
    for (auto &addr : addrs) {
        Socket::ptr sock = Socket::CreateTCPSocket(addr);
        if (!sock->bind(addr)) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "TCP SERVER BIND FAIL"
                                                          << "\n\terrno:" << errno
                                                          << "\n\terrstr:" << strerror(errno)
                                                          << "\n\taddr:" << addr->toString();
            rt = false;
            errAddrs.push_back(addr);
            continue;
        }
        if (!sock->listen()) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "TCP SERVER LISTEN FAIL"
                                                          << "\n\terrno:" << errno
                                                          << "\n\terrstr:" << strerror(errno)
                                                          << "\n\tsock:" << sock->toString()
                                                          << "\n\taddr:" << addr->toString();
            rt = false;
            errAddrs.push_back(addr);
            continue;
        }
        m_sockets.push_back(sock);
    }
    if (rt) {
        for (auto &sock : m_sockets) {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "SERVER BIND SUCCESS"
                                                         << sock->toString();
        }
    }
    return rt;
}

bool TcpServer::start() {
    if (!m_isStop) {
        return true;
    }
    m_isStop = false;
    for (auto &sock : m_sockets) {
        m_acceptIOM->schedule(std::bind(&TcpServer::startAccept, shared_from_this(), sock));
    }
    return true;
}

void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptIOM->schedule([this, self]() {
        for (auto &sock : m_sockets) {
            sock->cancelAll();
            sock->close();
        }
        m_sockets.clear();
    });
}

void TcpServer::startAccept(Socket::ptr sock) {
    while (!m_isStop) {
        Socket::ptr client = sock->accept();
        if (client) {
            client->setRecvTimeout(m_rdTimeout);
            m_workerIOM->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
        } else {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "ACCEPT ERROR:"
                                                          << "\n\terrno:" << errno
                                                          << "\n\tstrerror:" << strerror(errno);
        }
    }
}

void TcpServer::handleClient(Socket::ptr client) {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "handle client:\n\t" << client->toString();
}

}  // namespace tigerkin