/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_server.h"

namespace tigerkin {
namespace http {

HttpServer::HttpServer(bool keepalive, IOManager *workerIOM, IOManager *acceptIOM)
    : TcpServer(workerIOM, acceptIOM), m_keepalive(keepalive) {
}

void HttpServer::handleClient(Socket::ptr client) {
    HttpSession::ptr session(new HttpSession(client));
    do {
        HttpRequest::ptr req = session->recvRequest();
        if (!req) {
            TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "RECV REQUEST FAIL:"
                << "\n\terrno:" << errno
                << "\n\terrstr:" << strerror(errno)
                << "\n\tclient:" << client->toString();
            break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_keepalive));
        rsp->setBody("Hello! I'm tigerkin.");
        session->sendResponse(rsp);
    } while (m_keepalive);
    session->close();
}

}  // namespace http
}  // namespace tigerkin