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
    : TcpServer(workerIOM, acceptIOM), m_keepalive(keepalive), m_dispatch(new ServletDispatch) {
}

void HttpServer::handleClient(Socket::ptr client) {
    HttpSession::ptr session(new HttpSession(client));
    do {
        HttpRequest::ptr req = session->recvRequest();
        if (!req) {
            if (errno == ETIMEDOUT) {
                TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "Connection timed out localAddress:"
                                                             << client->getLocalAddress()->toString()
                                                             << "remoteAddress:" << client->getRemoteAddress()->toString();

            } else {
                TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "RECV REQUEST FAIL localAddress:" << client->getLocalAddress()->toString()
                                                             << " remoteAddress:" << client->getRemoteAddress()->toString()
                                                             << " errstr:" << strerror(errno);
            }
            break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_keepalive));
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);
        if (!m_keepalive || req->isClose()) {
            break;
        }
    } while (m_keepalive);
    session->close();
}

}  // namespace http
}  // namespace tigerkin