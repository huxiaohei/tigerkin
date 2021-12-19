/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "socket_stream.h"

namespace tigerkin {

SocketStream::SocketStream(Socket::ptr sock, bool owner)
    : m_socket(sock), m_owner(owner) {
}

SocketStream::~SocketStream() {
    if (m_owner) {
        close();
    }
}

ssize_t SocketStream::read(void *buffer, size_t length) {
    if (!isConnected()) {
        return -1;
    }
    return m_socket->recv(buffer, length);
}

ssize_t SocketStream::read(ByteArray::ptr ba, size_t length) {
    if (!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, length);
    int rt = m_socket->recv(&iovs[0], iovs.size());
    if (rt > 0) {
        ba->setOffset(ba->getOffset() + rt);
    }
    return rt;
}

ssize_t SocketStream::write(void *buffer, size_t length) {
    if (!isConnected()) {
        return -1;
    }
    return m_socket->recv(buffer, length);
}

ssize_t SocketStream::write(ByteArray::ptr ba, size_t length) {
    if (!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    ba->getReadBuffers(iovs, length);
    int rt = m_socket->send(&iovs[0], iovs.size());
    if (rt > 0) {
        ba->setOffset(ba->getOffset() + rt);
    }
    return rt;
}

void SocketStream::close() {
    if (m_socket) {
        m_socket->close();
    }
}

bool SocketStream::isConnected() {
    return m_socket && m_socket->isConnected();
}

}  // namespace tigerkin