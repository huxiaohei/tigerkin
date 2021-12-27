/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/09
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "socket.h"

#include "fdmanager.h"
#include "hook.h"
#include "iomanager.h"
#include "macro.h"
#include "netinet/tcp.h"

namespace tigerkin {

Socket::ptr Socket::CreateTCPSocket(Address::ptr addr) {
    Socket::ptr sock(new Socket(addr->getFamily(), SOCK_STREAM, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket(Address::ptr addr) {
    Socket::ptr sock(new Socket(addr->getFamily(), SOCK_DGRAM, 0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket() {
    Socket::ptr sock(new Socket(AF_INET, SOCK_STREAM, 0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket() {
    Socket::ptr sock(new Socket(AF_INET, SOCK_DGRAM, 0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6() {
    Socket::ptr sock(new Socket(AF_INET6, SOCK_STREAM, 0));
    return sock;
}

Socket::ptr CreateUDPSocket6() {
    Socket::ptr sock(new Socket(AF_INET6, SOCK_DGRAM, 0));
    return sock;
}

Socket::ptr CreateUnixTCPSocket() {
    Socket::ptr sock(new Socket(AF_UNIX, SOCK_STREAM, 0));
    return sock;
}

Socket::ptr CreateUnixUDPSocket() {
    Socket::ptr sock(new Socket(AF_UNIX, SOCK_DGRAM, 0));
    return sock;
}

Socket::Socket(int family, int type, int protocol)
    : m_socket(-1), m_family(family), m_type(type), m_protocol(protocol), m_isConnected(false) {
}

Socket::~Socket() {
    close();
}

int64_t Socket::getSendTimeout() const {
    FdEntity::ptr entity = SingletonFdMgr::GetInstance()->get(m_socket);
    if (entity) {
        return entity->getSendTimeout();
    }
    return -1;
}

void Socket::setSendTimeout(int64_t timeout) {
    struct timeval tv {
        int(timeout / 1000), int(timeout % 1000 * 1000)
    };
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int64_t Socket::getRecvTimeout() const {
    FdEntity::ptr entity = SingletonFdMgr::GetInstance()->get(m_socket);
    if (entity) {
        return entity->getRecvTimeout();
    }
    return -1;
}

void Socket::setRecvTimeout(int64_t timeout) {
    struct timeval tv {
        int(timeout / 1000), int(timeout % 1000 * 1000)
    };
    setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

int64_t Socket::getConnectTimeout() const {
    FdEntity::ptr entity = SingletonFdMgr::GetInstance()->get(m_socket);
    if (entity) {
        return entity->getConnectTimeout();
    }
    return -1;
}

bool Socket::getOption(int level, int optname, void *optval, socklen_t *optlen) const {
    int rt = ::getsockopt(m_socket, level, optname, optval, optlen);
    if (rt) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "GET OPTION WARN:\n\t"
                                                     << "level:" << level << "\n\t"
                                                     << "optname:" << optname << "\n\t"
                                                     << "errno:" << errno << "\n\t"
                                                     << "strerror:" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int optname, const void *optval, socklen_t optlen) {
    int rt = ::setsockopt(m_socket, level, optname, optval, optlen);
    if (rt) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "SET OPTION WARN:"
                                                     << "\n\tsocket:" << m_socket
                                                     << "\n\tfamily:" << m_family
                                                     << "\n\tlevel:" << level
                                                     << "\n\toptname:" << optname
                                                     << "\n\terrno:" << errno
                                                     << "\n\tstrerror:" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::bind(const Address::ptr addr) {
    if (!isValid()) {
        if (TIGERKIN_UNLIKELY(addr->getFamily() != m_family)) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "BIND ERROR:\n\t"
                                                          << "addr family:" << addr->getFamily() << "\n\t"
                                                          << "socket family:" << m_family;
            return false;
        }
        newSocket();
        if (TIGERKIN_UNLIKELY(!isValid())) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "BIND ERROR:\n\t"
                                                          << "add info:" << addr->toString() << "\n\t"
                                                          << "errno:" << errno << "\n\t"
                                                          << "strerror" << strerror(errno);
            return false;
        }
    }
    if (::bind(m_socket, addr->getAddr(), addr->getAddrLen())) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "BIND ERROR"
                                                      << "\n\terrno:" << errno
                                                      << "\n\tstrerror:" << strerror(errno);
        close();
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if (!isValid()) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "LISTEN ERROR:\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return false;
    }
    if (TIGERKIN_UNLIKELY(::listen(m_socket, backlog))) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "LISTEN ERROR:\n\t"
                                                      << "error:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept() {
    Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
    int newSocket = ::accept(m_socket, nullptr, nullptr);
    if (newSocket == -1) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "ACCEPT ERROR:\n\t"
                                                      << "family:" << m_family << "\n\t"
                                                      << "type:" << m_type << "\n\t"
                                                      << "protocol:" << m_protocol << "\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return nullptr;
    }
    if (sock->init(newSocket)) {
        return sock;
    }
    return nullptr;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout) {
    if (!isValid()) {
        if (TIGERKIN_UNLIKELY(addr->getFamily() != m_family)) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "CONNECT ERROR:\n\t"
                                                          << "addr family:" << addr->getFamily() << "\n\t"
                                                          << "socket family:" << m_family;
            return false;
        }
        newSocket();
        if (TIGERKIN_UNLIKELY(!isValid())) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "CONNECT ERROR:\n\t"
                                                          << "add info:" << addr->toString() << "\n\t"
                                                          << "errno:" << errno << "\n\t"
                                                          << "strerror" << strerror(errno);
            return false;
        }
    }
    FdEntity::ptr entity = SingletonFdMgr::GetInstance()->get(m_socket);
    if (TIGERKIN_LIKELY(entity)) {
        entity->setConnectTimeout(timeout);
    }
    if (::connect(m_socket, addr->getAddr(), addr->getAddrLen())) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "CONNECT ERROR:\n\t"
                                                      << "add info:" << addr->toString() << "\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        close();
        return false;
    }
    m_isConnected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}

Address::ptr Socket::getRemoteAddress() {
    if (m_remoteAddress) {
        return m_remoteAddress;
    }
    Address::ptr rst;
    switch (m_family) {
        case AF_INET:
            rst.reset(new IpV4Address());
            break;
        case AF_INET6:
            rst.reset(new IpV6Address());
            break;
        case AF_UNIX:
            rst.reset(new UnixAddress());
            break;
        default:
            rst.reset(new UnknowAddress(m_family));
            break;
    }
    socklen_t addrLen = rst->getAddrLen();
    if (getpeername(m_socket, rst->getAddr(), &addrLen)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "GET REMOTE ADDRESS ERROR:\n\t"
                                                      << "address info:" << rst->toString() << "\n\t"
                                                      << "family:" << m_family << "\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror:" << strerror(errno);
        return Address::ptr(new UnknowAddress(m_family));
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(rst);
        addr->setAddrLen(addrLen);
    }
    m_remoteAddress = rst;
    return m_remoteAddress;
}

Address::ptr Socket::getLocalAddress() {
    if (m_localAddress) {
        return m_localAddress;
    }
    Address::ptr rst;
    switch (m_family) {
        case AF_INET:
            rst.reset(new IpV4Address());
            break;
        case AF_INET6:
            rst.reset(new IpV6Address());
            break;
        case AF_UNIX:
            rst.reset(new UnixAddress());
            break;
        default:
            rst.reset(new UnknowAddress(m_family));
            break;
    }
    socklen_t addrLen = rst->getAddrLen();
    if (getsockname(m_socket, rst->getAddr(), &addrLen)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "GET LOCAL ADDRESS ERROR:\n\t"
                                                      << "address info:" << rst->toString() << "\n\t"
                                                      << "family:" << m_family << "\n\t"
                                                      << "errno:" << errno << "\n\t"
                                                      << "strerror" << strerror(errno);
        return UnknowAddress::ptr(new UnknowAddress(m_family));
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(rst);
        addr->setAddrLen(addrLen);
    }
    m_localAddress = rst;
    return m_localAddress;
}

bool Socket::init(int sock) {
    FdEntity::ptr entity = SingletonFdMgr::GetInstance()->get(sock);
    if (entity && entity->isSocket() && !entity->isClosed()) {
        m_socket = sock;
        m_isConnected = true;
        initSocket();
        getRemoteAddress();
        getLocalAddress();
        return true;
    }
    return false;
}

void Socket::initSocket() {
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);
    if (m_type == SOCK_STREAM && m_family != AF_UNIX) {
        setOption(IPPROTO_TCP, TCP_NODELAY, val);
    }
}

void Socket::newSocket() {
    m_socket = ::socket(m_family, m_type, m_protocol);
    if (TIGERKIN_LIKELY(m_socket >= 0)) {
        initSocket();
    } else {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "NEW SOCKET ERROR:\n\t"
                                                      << "family:" << m_family << "\n\t"
                                                      << "type:" << m_type << "\n\t"
                                                      << "protocol:" << m_protocol << "\n\t";
    }
}

bool Socket::close() {
    if (!isConnected() && m_socket == -1) {
        return true;
    }
    m_isConnected = false;
    if (m_socket != -1) {
        ::close(m_socket);
        m_socket = -1;
    }
    return true;
}

ssize_t Socket::send(const void *buffer, size_t length, int flags) {
    if (TIGERKIN_LIKELY(isConnected())) {
        return ::send(m_socket, buffer, length, flags);
    }
    return -1;
}

ssize_t Socket::send(const iovec *buffers, size_t length, int flags) {
    if (TIGERKIN_LIKELY(isConnected())) {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec *)buffers;
        msg.msg_iovlen = length;
        msg.msg_flags = flags;
        return ::sendmsg(m_socket, &msg, flags);
    }
    return -1;
}

ssize_t Socket::sendTo(const void *buffer, size_t length, Address::ptr to, int flags) {
    if (TIGERKIN_UNLIKELY(!isValid())) {
        return -1;
    }
    return ::sendto(m_socket, buffer, length, flags, to->getAddr(), to->getAddrLen());
}

ssize_t Socket::sendTo(const iovec *buffers, size_t length, Address::ptr to, int flags) {
    if (TIGERKIN_UNLIKELY(!isValid())) {
        return -1;
    }
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec *)buffers;
    msg.msg_iovlen = length;
    msg.msg_flags = flags;
    msg.msg_name = to->getAddr();
    msg.msg_namelen = to->getAddrLen();
    return ::sendmsg(m_socket, &msg, flags);
}

ssize_t Socket::recv(void *buffer, size_t length, int flags) {
    if (TIGERKIN_LIKELY(isConnected())) {
        return ::recv(m_socket, buffer, length, flags);
    }
    return -1;
}

ssize_t Socket::recv(iovec *buffers, size_t length, int flags) {
    if (TIGERKIN_LIKELY(isConnected())) {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec *)buffers;
        msg.msg_iovlen = length;
        msg.msg_flags = flags;
        return ::recvmsg(m_socket, &msg, flags);
    }
    return -1;
}

ssize_t Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags) {
    if (TIGERKIN_UNLIKELY(!isValid())) {
        return -1;
    }
    socklen_t len = from->getAddrLen();
    return ::recvfrom(m_socket, buffer, length, flags, from->getAddr(), &len);
}

ssize_t Socket::recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags) {
    if (TIGERKIN_UNLIKELY(!isValid())) {
        return -1;
    }
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec *)buffers;
    msg.msg_iovlen = length;
    msg.msg_flags = flags;
    msg.msg_name = from->getAddr();
    msg.msg_namelen = from->getAddrLen();
    return ::recvmsg(m_socket, &msg, flags);
}

int Socket::getError() const {
    int error = -1;
    socklen_t len = sizeof(error);
    if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    return error;
}

bool Socket::cancelAccept() {
    return IOManager::GetThis()->cancelEvent(m_socket, IOManager::Event::READ);
}

bool Socket::cancelRead() {
    return IOManager::GetThis()->cancelEvent(m_socket, IOManager::Event::READ);
}

bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_socket, IOManager::Event::WRITE);
}

bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAllEvent(m_socket);
}

std::ostream &Socket::dump(std::ostream &os) const {
    os << "socket:" << m_socket
       << "\nfamily:" << m_family
       << "\ntype:" << m_type
       << "\nprotocol:" << m_protocol
       << "\nisConnected:" << m_isConnected;
    if (m_localAddress) {
        os << "\nlocalAddress:" << m_localAddress->toString();
    }
    if (m_remoteAddress) {
        os << "\nremoteAddress:" << m_remoteAddress->toString();
    }
    return os;
}

std::string Socket::toString() const {
    std::stringstream os;
    dump(os);
    return os.str();
}

}  // namespace tigerkin