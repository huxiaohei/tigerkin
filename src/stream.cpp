/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "stream.h"

namespace tigerkin {

ssize_t Stream::readFixLength(void *buffer, size_t length) {
    size_t left = length;
    size_t offset = 0;
    while (left > 0) {
        ssize_t len = read((char *)buffer + offset, left);
        if (len < 0) {
            return -1;
        }
        left -= len;
        offset += len;
    }
    return length;
}

ssize_t Stream::readFixLength(ByteArray::ptr ba, size_t length) {
    size_t left = length;
    while (left > 0) {
        ssize_t len = read(ba, left);
        if (len < 0) {
            return -1;
        }
        left -= len;
    }
    return length;
}

ssize_t Stream::writeFixLength(const void *buffer, size_t length) {
    size_t left = length;
    size_t offset = 0;
    while (left > 0) {
        ssize_t len = write((const char *)buffer + offset, left);
        if (len < 0) {
            return -1;
        }
        left -= len;
        offset += len;
    }
    return length;
}

ssize_t Stream::writeFixLength(ByteArray::ptr ba, size_t length) {
    size_t left = length;
    while (left > 0) {
        ssize_t len = write(ba, left);
        if (len < 0) {
            return -1;
        }
        left -= len;
    }
    return length;
}

}  // namespace tigerkin