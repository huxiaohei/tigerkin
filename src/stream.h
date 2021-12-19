/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_STREAM_H__
#define __TIGERKIN_STREAM_H__

#include "bytearray.h"
#include "socket.h"

namespace tigerkin {

class Stream {
   public:

    virtual ~Stream() {}

    virtual ssize_t read(void *buffer, size_t length) = 0;
    virtual ssize_t read(ByteArray::ptr ba, size_t length) = 0;
    virtual ssize_t readFixLength(void *buffer, size_t length);
    virtual ssize_t readFixLength(ByteArray::ptr ba, size_t length);

    virtual ssize_t write(void *buffer, size_t length) = 0;
    virtual ssize_t write(ByteArray::ptr ba, size_t length) = 0;
    virtual ssize_t writeFixLength(const void *buffer, size_t length);
    virtual ssize_t writeFixLength(ByteArray::ptr ba, size_t length);

    virtual void close() = 0;
};

}  // namespace tigerkin

#endif  // !__TIGERKIN_STREAM_H__