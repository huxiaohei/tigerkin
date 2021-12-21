/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HTTP_SERVER_H__
#define __TIGERKIN_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_session.h"

namespace tigerkin {
namespace http {

class HttpServer : public TcpServer {
   public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false, IOManager *workerIOM = IOManager::GetThis(), IOManager *acceptIOM = IOManager::GetThis());

   protected:
    void handleClient(Socket::ptr client) override;

   private:
    bool m_keepalive;
};

}  // namespace http
}  // namespace tigerkin

#endif  // !__TIGERKIN_HTTP_SERVER_H__