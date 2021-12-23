/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HTTP_SERVER_H__
#define __TIGERKIN_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_servlet.h"
#include "http_session.h"

namespace tigerkin {
namespace http {

class HttpServer : public TcpServer {
   public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keepalive = false, IOManager *workerIOM = IOManager::GetThis(), IOManager *acceptIOM = IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }

   protected:
    void handleClient(Socket::ptr client) override;

   private:
    bool m_keepalive;
    ServletDispatch::ptr m_dispatch;
};

}  // namespace http
}  // namespace tigerkin

#endif  // !__TIGERKIN_HTTP_SERVER_H__