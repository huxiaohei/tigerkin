/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/12/05
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_TCP_SERVER_H__
#define __TIGERKIN_TCP_SERVER_H__

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "address.h"
#include "iomanager.h"
#include "socket.h"

namespace tigerkin {

class TcpServer : public std::enable_shared_from_this<TcpServer> {
   public:
    typedef std::shared_ptr<TcpServer> ptr;

    TcpServer(IOManager *workerIOM = IOManager::GetThis(), IOManager *acceptIOM = IOManager::GetThis());
    ~TcpServer();

    virtual bool bind(Address::ptr addr);
    virtual bool bind(std::vector<Address::ptr> addrs, std::vector<Address::ptr> &errAddrs);
    virtual bool start();
    virtual void stop();

    uint64_t getReadTimeout() const { return m_rdTimeout; }
    void setReadTimeout(uint64_t v) { m_rdTimeout = v; }

    std::string getName() const { return m_name; }
    void setName(std::string v) { m_name = v; }
    bool isStop() const { return m_isStop; }

   protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);

   private:
    std::vector<Socket::ptr> m_sockets;
    IOManager *m_workerIOM;
    IOManager *m_acceptIOM;
    uint64_t m_rdTimeout;
    std::string m_name;
    bool m_isStop;
};

}  // namespace tigerkin

#endif  // !_TIGERKIN_TCP_SERVER_H__