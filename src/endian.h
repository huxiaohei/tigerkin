/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/06
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_ENDIAN_H__
#define __TIGERKIN_ENDIAN_H__

#define TIGERKIN_LITTLE_ENDIAN 1
#define TIGERKIN_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

#include <iostream>

namespace tigerkin {

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64(value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32(value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16(value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define TIGERKIN_BYTE_ORDER TIGERKIN_BIG_ENDIAN
#else
#define TIGERKIN_BYTE_ORDER TIGERKIN_LITTLE_ENDIAN
#endif

#if TIGERKIN_BYTE_ORDER == TIGERKIN_BIG_ENDIAN

template <class T>
T byteswapOnLittleEndian(T v) {
    return v;
}

template <class T>
T byteswapOnBigEndian(T v) {
    return byteswap(v);
}

#endif

#if TIGERKIN_BYTE_ORDER == TIGERKIN_LITTLE_ENDIAN

template <class T>
T byteswapOnLittleEndian(T v) {
    return byteswap(v);
}

template <class T>
T byteswapOnBigEndian(T v) {
    return v;
}

#endif

}  // namespace tigerkin

#endif  // !__TIGERKIN_ENDIAN_H__