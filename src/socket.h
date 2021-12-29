/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/07
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_SOCKET_H__
#define __TIGERKIN_SOCKET_H__

#include <memory>

#include "address.h"
#include "unistd.h"

namespace tigerkin {

class Socket : std::enable_shared_from_this<Socket> {
   public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weakPtr;

    static Socket::ptr CreateTCPSocket(Address::ptr addr);
    static Socket::ptr CreateUDPSocket(Address::ptr addr);
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();
    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();
    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();

    Socket(int family, int type, int protocol = 0);
    ~Socket();

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    int getFamily() const { return m_family; };
    int getType() const { return m_type; };
    int getProtocol() const { return m_protocol; };
    int getError() const;
    int64_t getSendTimeout() const;
    void setSendTimeout(int64_t timeout);
    int64_t getRecvTimeout() const;
    void setRecvTimeout(int64_t timeout);
    int64_t getConnectTimeout() const;

    bool getOption(int level, int optname, void *optval, socklen_t *optlen) const;
    template <class T>
    bool getOption(int level, int optname, T &result) const {
        size_t len = sizeof(T);
        return getOption(level, optname, &result, &len);
    }

    bool setOption(int level, int optname, const void *optval, socklen_t optlen);
    template <class T>
    bool setOption(int level, int optname, T &result) {
        size_t len = sizeof(T);
        return setOption(level, optname, &result, len);
    }

    virtual std::ostream &dump(std::ostream &os) const;
    virtual std::string toString() const;
    bool isConnected() const { return m_isConnected; }
    bool hasClosed() const { return m_hasClosed; }
    bool isValid() const { return m_socket != -1; }

    virtual bool bind(const Address::ptr addr);
    virtual bool listen(int backlog = SOMAXCONN);
    virtual Socket::ptr accept();
    virtual bool connect(const Address::ptr addr, uint64_t timeout = -1);
    virtual bool close();

    virtual ssize_t send(const void *buffer, size_t length, int flags = 0);
    virtual ssize_t send(const iovec *buffers, size_t length, int flags = 0);
    virtual ssize_t sendTo(const void *buffer, size_t length, Address::ptr to, int flags = 0);
    virtual ssize_t sendTo(const iovec *buffers, size_t length, Address::ptr to, int flags = 0);

    virtual ssize_t recv(void *buffer, size_t length, int flags = 0);
    virtual ssize_t recv(iovec *buffers, size_t length, int flags = 0);
    virtual ssize_t recvFrom(void *buffer, size_t length, Address::ptr from, int flags = 0);
    virtual ssize_t recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags = 0);

    bool cancelAccept();
    bool cancelRead();
    bool cancelWrite();
    bool cancelAll();

   protected:
    void initSocket();
    void newSocket();
    virtual bool init(int fd);

   private:
    int m_socket;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_isConnected;
    bool m_hasClosed;
    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;
};

class SSLSocket : public Socket {
    // todo
   public:
    typedef std::shared_ptr<SSLSocket> ptr;
};

}  // namespace tigerkin

#endif  // !__TIGERKIN_SOCKET_H__