#ifndef __TIGERKIN_MACRO_H__
#define __TIGERKIN_MACRO_H__

#include <assert.h>
#include <string.h>

#include "log.h"
#include "util.h"

#define TIGERKIN_ASSERT(x)                                                                            \
    if (!(x)) {                                                                                       \
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "\nASSERTION: " #x                           \
                                                      << "\nbacktrace:"                               \
                                                      << tigerkin::BacktraceToString(100, 2, "    "); \
        assert(x);                                                                                    \
    }

#define TIGERKIN_ASSERT2(x, w)                                                                        \
    if (!(x)) {                                                                                       \
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "\nASSERTION: " #x << "    " << w            \
                                                      << "\nbacktrace:\n"                             \
                                                      << tigerkin::BacktraceToString(100, 2, "    "); \
    }

#endif