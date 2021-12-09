/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/31
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_FDMANAGER_H__
#define __TIGERKIN_FDMANAGER_H__

#include <memory>
#include <vector>

#include "mutex.h"
#include "singleton.h"
#include "timer.h"

namespace tigerkin {

class FdEntity : public std::enable_shared_from_this<FdEntity> {
   public:
    typedef std::shared_ptr<FdEntity> ptr;

    FdEntity(int fd);
    ~FdEntity();

    bool init();
    bool isInit() const { return m_isInit; }
    bool isSysNonblock() const { return m_isSysNonblock; }
    bool isUserNonblock() const { return m_isUserNonblock; }
    bool isSocket() const { return m_isSocket; }
    bool isClosed() const { return m_isClosed; }
    int getFd() const { return m_fd; }
    uint64_t getRecvTimeout() const { return m_recvTimeout; }
    uint64_t getSendTimeout() const { return m_sendTimeout; }
    uint64_t getConnectTimeout() const;
    Timer::ptr getTimer() const { return m_timer; }

    void setSysNonblock(bool flag) { m_isSysNonblock = flag; }
    void setUserNonblock(bool flag) { m_isUserNonblock = flag; }
    void setRecvTimeout(uint64_t timeout) { m_recvTimeout = timeout; }
    void setSendTimeout(uint64_t timeout) { m_sendTimeout = timeout; }
    void setConnectTimeout(uint64_t timeout) { m_connectTimeout = timeout; }
    void setTimer(Timer::ptr timer) { m_timer = timer; }

   private:
    bool m_isInit = false;
    bool m_isSysNonblock = false;
    bool m_isUserNonblock = false;
    bool m_isSocket = false;
    bool m_isClosed = false;
    int m_fd = 0;
    uint64_t m_recvTimeout = 0;
    uint64_t m_sendTimeout = 0;
    uint64_t m_connectTimeout = 0;
    Timer::ptr m_timer = nullptr;
};

class FdManager : public std::enable_shared_from_this<FdManager> {
   public:
    typedef std::shared_ptr<FdManager> ptr;
    typedef ReadWriteLock Lock;

    FdManager();
    ~FdManager();

    FdEntity::ptr get(int fd, bool autoCreate = false);
    FdEntity::ptr add(int fd);
    void del(int fd);

   private:
    Lock m_mutex;
    std::vector<FdEntity::ptr> m_data;
};

typedef tigerkin::Singleton<FdManager> SingletonFdMgr;

}  // namespace tigerkin

#endif
