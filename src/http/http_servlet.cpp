/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/23
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "http_servlet.h"

#include <fnmatch.h>

namespace tigerkin {

namespace http {

Servlet::Servlet(const std::string name)
    : m_name(name) {
}

NotFoundServlet::NotFoundServlet()
    : Servlet("NotFoundServlet") {
}

int32_t NotFoundServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) {
    static const std::string &rspBody =
        "<html>\
            <head>\
                <title>\
                    404 Not Found\
                </title>\
            </head>\
            <body>\
                <center>\
                    <h1>404 Not Found</h1>\
                </center>\
            </body>\
        </html>";
    response->setStatus(HttpStatus::NOT_FOUND);
    response->setHeader("Content-Type", "text/html");
    response->setHeader("Server", "tigerkin/1.0.0");
    response->setBody(rspBody);
    return 0;
}

FunctionServlet::FunctionServlet(Callback cb)
    : Servlet("FunctionServlet"), m_cb(cb) {
}

int32_t FunctionServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) {
    return m_cb(request, response, session);
}

ServletDispatch::ServletDispatch()
    : Servlet("ServletDispatch") {
    m_defaultServlet.reset(new NotFoundServlet);
}

int32_t ServletDispatch::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) {
    Servlet::ptr slt = getMatchedServlet(request->getPath());
    if (slt) {
        slt->handle(request, response, session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string &path, Servlet::ptr slt) {
    ReadWriteLock::WriteLock lock(m_rdLock);
    m_datas[path] = slt;
}

void ServletDispatch::addServlet(const std::string &path, FunctionServlet::Callback cb) {
    ReadWriteLock::WriteLock lock(m_rdLock);
    m_datas[path].reset(new FunctionServlet(cb));
}

void ServletDispatch::addGlobServlet(const std::string &path, Servlet::ptr slt) {
    ReadWriteLock::WriteLock lock(m_rdLock);
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator it = m_globs.begin();
    while (it != m_globs.end()) {
        if (it->first == path) {
            m_globs.erase(it);
        }
    }
    m_globs.push_back(std::make_pair(path, slt));
}

void ServletDispatch::addGlobServlet(const std::string &path, FunctionServlet::Callback cb) {
    addGlobServlet(path, FunctionServlet::ptr(new FunctionServlet(cb)));
}

void ServletDispatch::delServlet(const std::string &path) {
    ReadWriteLock::WriteLock lock(m_rdLock);
    m_datas.erase(path);
}

void ServletDispatch::delGlobServlet(const std::string &path) {
    ReadWriteLock::WriteLock lock(m_rdLock);
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator it = m_globs.begin();
    while (it != m_globs.end()) {
        if (it->first == path) {
            m_globs.erase(it);
            break;
        }
    }
}

void ServletDispatch::setDefaultServlet(Servlet::ptr defaultServlet) {
    m_defaultServlet = defaultServlet;
}

Servlet::ptr ServletDispatch::getDefaultServlet() {
    return m_defaultServlet;
}

Servlet::ptr ServletDispatch::getServlet(const std::string &path) {
    ReadWriteLock::ReadLock lock(m_rdLock);
    std::unordered_map<std::string, Servlet::ptr>::iterator it = m_datas.find(path);
    return it == m_datas.end() ? nullptr : it->second;
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string &path) {
    ReadWriteLock::ReadLock lock(m_rdLock);
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator it = m_globs.begin();
    while (it != m_globs.end()) {
        if (it->first == path) {
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string &path) {
    ReadWriteLock::ReadLock lock(m_rdLock);
    std::unordered_map<std::string, Servlet::ptr>::iterator mIt = m_datas.find(path);
    if (mIt != m_datas.end()) {
        return mIt->second;
    }
    std::vector<std::pair<std::string, Servlet::ptr>>::iterator gIt = m_globs.begin();
    while (gIt != m_globs.end()) {
        if (!fnmatch(gIt->first.c_str(), path.c_str(), 0)) {
            return gIt->second;
        }
    }
    return m_defaultServlet;
}

}  // namespace http
}  // namespace tigerkin