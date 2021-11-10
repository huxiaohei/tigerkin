/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/31
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "fdmanager.h"

#include <sys/stat.h>

#include <iostream>

#include "config.h"
#include "hook.h"

namespace tigerkin {

static ConfigVar<uint64_t>::ptr g_tcp_connect_timeout = Config::Lookup<uint64_t>("tigerkin.socket.tcpConnectTimeout", 6000, "socket timeout");

FdEntity::FdEntity(int fd)
    : m_isInit(false), m_isSysNonblock(false), m_isUserNonblock(false), m_isSocket(false), m_isClosed(false), m_fd(fd), m_timer(nullptr) {
    init();
}

FdEntity::~FdEntity() {
}

uint64_t FdEntity::getConnectTimeout() const {
    if (m_connectTimeout < 0) {
        return g_tcp_connect_timeout->getValue();
    }
    return m_connectTimeout;
}

bool FdEntity::init() {
    if (m_isInit) {
        return m_isInit;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    struct stat fdStat;
    if (-1 == fstat(m_fd, &fdStat)) {
        m_isInit = false;
        m_isSocket = false;
        m_isClosed = true;
    } else {
        m_isInit = true;
        m_isSocket = S_ISSOCK(fdStat.st_mode);
    }
    if (m_isSocket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK)) {
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_isSysNonblock = true;
        m_isClosed = false;
    } else {
        m_isSysNonblock = false;
        m_isClosed = false;
    }
    m_isUserNonblock = false;
    return m_isInit;
}

FdManager::FdManager() {
    m_data.resize(60);
}

FdManager::~FdManager() {
}

FdEntity::ptr FdManager::get(int fd, bool autoCreate) {
    {
        Lock::ReadLock rLock(m_mutex);
        if (fd >= (int)m_data.size()) {
            if (!autoCreate) {
                return nullptr;
            }
        } else {
            if (m_data[fd]) {
                return m_data[fd];
            }
            if (!autoCreate) {
                return nullptr;
            }
        }
    }
    return add(fd);
}

FdEntity::ptr FdManager::add(int fd) {
    Lock::WriteLock wLock(m_mutex);
    if (fd >= (int)m_data.size()) {
        m_data.resize((m_data.size() * 1.5) > fd ? m_data.size() * 1.5 : fd);
    }
    FdEntity::ptr fdEntity(new FdEntity(fd));
    m_data[fd] = fdEntity;
    return fdEntity;
}

void FdManager::del(int fd) {
    Lock::WriteLock wLock(m_mutex);
    if (fd >= (int)m_data.size()) {
        return;
    }
    m_data[fd].reset();
}

}  // namespace tigerkin