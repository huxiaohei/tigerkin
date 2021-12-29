/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/23
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_HTTP_SERVLET_H__
#define __TIGERKIN_HTTP_SERVLET_H__

#include <functional>
#include <unordered_map>

#include "../mutex.h"
#include "http.h"
#include "http_session.h"

namespace tigerkin {
namespace http {

class Servlet {
   public:
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(const std::string name);
    virtual ~Servlet(){};
    virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) = 0;

    const std::string &getName() { return m_name; }

   protected:
    std::string m_name;
};

class FunctionServlet : public Servlet {
   public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session)> Callback;

    FunctionServlet(Callback cb);
    int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;

   private:
    Callback m_cb;
};

class NotFoundServlet : public Servlet {
   public:
    typedef std::shared_ptr<NotFoundServlet> ptr;

    NotFoundServlet();
    int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;
};

class ServletDispatch : public Servlet {
   public:
    typedef std::shared_ptr<ServletDispatch> ptr;

    ServletDispatch();

    int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;

    void addServlet(const std::string &path, Servlet::ptr slt);
    void addServlet(const std::string &path, FunctionServlet::Callback cb);
    void addGlobServlet(const std::string &path, Servlet::ptr slt);
    void addGlobServlet(const std::string &path, FunctionServlet::Callback cb);

    void delServlet(const std::string &path);
    void delGlobServlet(const std::string &path);

    void setDefaultServlet(Servlet::ptr defaultServlet);
    Servlet::ptr getDefaultServlet();
    Servlet::ptr getServlet(const std::string &path);
    Servlet::ptr getGlobServlet(const std::string &path);
    Servlet::ptr getMatchedServlet(const std::string &path);

   private:
    ReadWriteLock m_rdLock;
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    std::vector<std::pair<std::string, Servlet::ptr>> m_globs;
    Servlet::ptr m_defaultServlet;
};

}  // namespace http
}  // namespace tigerkin

#endif  // !__TIGERKIN_HTTP_SERVLET_H__