/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_SOCKET_STREAM_H__
#define __TIGERKIN_SOCKET_STREAM_H__

#include "../stream.h"

namespace tigerkin {

class SocketStream : public Stream {
   public:
    typedef std::shared_ptr<SocketStream> ptr;

    SocketStream(Socket::ptr sock, bool owner);
    ~SocketStream();

    Socket::ptr getSocket() const { return m_socket; }

    ssize_t read(void *buffer, size_t length) override;
    ssize_t read(ByteArray::ptr ba, size_t length) override;
    ssize_t write(const void *buffer, size_t length) override;
    ssize_t write(ByteArray::ptr ba, size_t length) override;
    void close() override;

    bool isConnected();

   protected:
    Socket::ptr m_socket;
    bool m_owner;
};

}  // namespace tigerkin

#endif  // !__TIGERKIN_SOCKET_STREAM_H__