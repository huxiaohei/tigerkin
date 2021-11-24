/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/11/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include <sys/types.h>

typedef void (*element_cb)(void *data, const char *at, size_t length);
typedef void (*field_cb)(void *data, const char *field, size_t flen, const char *value, size_t vlen);

#endif  // !__HTTP_PARSER_H__