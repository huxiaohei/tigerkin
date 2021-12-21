/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/19
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HTTP_SESSION_H__
#define __TIGERKIN_HTTP_SESSION_H__

#include "../stream/socket_stream.h"
#include "http_parser.h"

namespace tigerkin {

namespace http {

class HttpSession : public SocketStream {
   public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(Socket::ptr socket, bool owner = true);
    HttpRequest::ptr recvRequest();
    ssize_t sendResponse(HttpResponse::ptr rsp);
};

}  // namespace http

}  // namespace tigerkin

#endif  // !__TIGERKIN_HTTP_SESSION_H__