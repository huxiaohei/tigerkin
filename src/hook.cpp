/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/24
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "hook.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>

#include "fdmanager.h"
#include "iomamager.h"
#include "macro.h"

namespace tigerkin {

static ConfigVar<uint16_t>::ptr g_tcp_connect_timeout = Config::Lookup<uint16_t>("tigerkin.socket.tcpConnectTimeout", 6000, "socket timeout");
static thread_local bool t_enable_hook = false;

#define HOOK_FUNC(XX) \
    XX(sleep);        \
    XX(usleep);       \
    XX(nanosleep);    \
    XX(socket);       \
    XX(connect);      \
    XX(accept);       \
    XX(read);         \
    XX(readv);        \
    XX(recv);         \
    XX(recvfrom);     \
    XX(recvmsg);      \
    XX(write);        \
    XX(writev);       \
    XX(send);         \
    XX(sendto);       \
    XX(sendmsg);      \
    XX(close);        \
    XX(fcntl);        \
    XX(ioctl);        \
    XX(getsockopt);   \
    XX(setsockopt)

void hookInit() {
    static bool isInit = false;
    if (isInit) {
        return;
    }
    isInit = true;
#define XX(name) name##_f = (name##_func)dlsym(RTLD_NEXT, #name);
    HOOK_FUNC(XX);
#undef XX
}

struct _HookIniter {
    _HookIniter() {
        hookInit();
    }
};
static _HookIniter s_hookIniter;

void setEnableHook(bool enable) {
    t_enable_hook = enable;
}

bool isEnableHook() {
    return t_enable_hook;
}

}  // namespace tigerkin

typedef struct {
    bool canceled = false;
} SocketIoState;

template <typename OriginFunc, typename... Args>
static ssize_t doSocketIo(int fd, OriginFunc func, const char *hookFucName, tigerkin::IOManager::Event event, Args &&...args) {
    if (!tigerkin::t_enable_hook) {
        return func(fd, std::forward<Args>(args)...);
    }
    tigerkin::FdEntity::ptr fdEntity = tigerkin::SingletonFdMgr::GetInstance()->get(fd);
    if (!fdEntity) {
        return func(fd, std::forward<Args>(args)...);
    }
    if (fdEntity->isClosed()) {
        errno = EBADF;
        return -1;
    }
    if (!fdEntity->isSocket() || fdEntity->isUserNonblock()) {
        return func(fd, std::forward<Args>(args)...);
    }
    int timeout = -1;
    if (event & tigerkin::IOManager::Event::READ) {
        timeout = fdEntity->getRecvTimeout();
    } else if (event & tigerkin::IOManager::Event::WRITE) {
        timeout = fdEntity->getSendTimeout();
    } else {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "IOMANAGER EVENT NOT FOUND \n\t"
                                                     << "event:" << event << "\n\t"
                                                     << "hookFucName:" << hookFucName;
    }

    std::shared_ptr<SocketIoState> state(new SocketIoState);
    ssize_t n = -1;

    do {
        n = func(fd, std::forward<Args>(args)...);
        if (n == -1 && errno == EAGAIN) {
            tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
            tigerkin::Timer::ptr timer;
            std::weak_ptr<SocketIoState> weekState(state);
            if (timeout >= 0) {
                timer = iom->addCondTimer(
                    timeout, [weekState, fd, iom, event]() {
                        std::shared_ptr<SocketIoState> t = weekState.lock();
                        if (!t || t->canceled) {
                            return;
                        }
                        t->canceled = true;
                        iom->cancelEvent(fd, event);
                    },
                    weekState);
            }
            if (iom->addEvent(fd, event) == tigerkin::IOManager::ADD_EVENT_SUC) {
                tigerkin::Coroutine::Yield();
                if (timer) {
                    timer->cancel();
                }
                if (state->canceled) {
                    errno = ETIMEDOUT;
                    return -1;
                }
                state->canceled = false;
            } else {
                if (timer) {
                    timer->cancel();
                }
                TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "IOMANAGER ADD EVENT FAIL \n\t"
                                                             << "event:" << event << "\n\t"
                                                             << "hookFucName:" << hookFucName;
                return -1;
            }
        }
    } while (n == -1 || errno == EINTR);
    return n;
}

extern "C" {
#define XX(name) name##_func name##_f = nullptr;
HOOK_FUNC(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!tigerkin::isEnableHook()) {
        return sleep_f(seconds);
    }
    tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
    tigerkin::Coroutine::ptr co = tigerkin::Coroutine::GetThis();
    iom->addTimer(
        seconds * 1000, [co, iom]() {
            iom->schedule(co);
        },
        false);
    tigerkin::Coroutine::Yield();
    return 0;
}

int usleep(useconds_t usec) {
    if (!tigerkin::isEnableHook()) {
        return usleep(usec);
    }
    tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
    tigerkin::Coroutine::ptr co = tigerkin::Coroutine::GetThis();
    iom->addTimer(
        usec / 1000, [co, iom]() {
            iom->schedule(co);
        },
        false);
    tigerkin::Coroutine::Yield();
    return 0;
}

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp) {
    if (!tigerkin::isEnableHook()) {
        return nanosleep_f(rqtp, rmtp);
    }
    int ms = rqtp->tv_sec * 1000 + rqtp->tv_nsec / 1000000;
    tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
    tigerkin::Coroutine::ptr co = tigerkin::Coroutine::GetThis();
    iom->addTimer(
        ms, [co, iom]() {
            iom->schedule(co);
        },
        false);
    tigerkin::Coroutine::Yield();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if (!tigerkin::isEnableHook()) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if (fd == -1) {
        TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "create socket error";
        return fd;
    }
    return tigerkin::SingletonFdMgr::GetInstance()->get(fd, true)->getFd();
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (!tigerkin::isEnableHook()) {
        return connect_f(sockfd, addr, addrlen);
    }
    tigerkin::FdEntity::ptr fdEntity = tigerkin::SingletonFdMgr::GetInstance()->get(sockfd);
    if (!fdEntity || fdEntity->isClosed()) {
        errno = EBADF;
        return -1;
    }
    if (!fdEntity->isSocket()) {
        return connect_f(sockfd, addr, addrlen);
    }
    if (fdEntity->isUserNonblock()) {
        return connect_f(sockfd, addr, addrlen);
    }
    int n = connect_f(sockfd, addr, addrlen);
    if (n == 0) {
        return n;
    } else if (n != -1 || errno != EINPROGRESS) {
        return n;
    }

    tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
    std::shared_ptr<SocketIoState> state(new SocketIoState);
    std::weak_ptr<SocketIoState> weekState(state);
    tigerkin::Timer::ptr timer;
    if (tigerkin::g_tcp_connect_timeout->getValue() > 0) {
        timer = iom->addCondTimer(
            tigerkin::g_tcp_connect_timeout->getValue(), [weekState, sockfd, iom]() {
                std::shared_ptr<SocketIoState> t = weekState.lock();
                if (!t || t->canceled) {
                    return;
                }
                t->canceled = true;
                iom->cancelEvent(sockfd, tigerkin::IOManager::Event::WRITE);
            },
            weekState);
    }
    if (iom->addEvent(sockfd, tigerkin::IOManager::Event::WRITE) == tigerkin::IOManager::ADD_EVENT_SUC) {
        tigerkin::Coroutine::Yield();
        if (timer) {
            timer->cancel();
        }
        if (state->canceled) {
            errno = ETIMEDOUT;
            return -1;
        }
    } else {
        if (timer) {
            timer->cancel();
            state->canceled = true;
        }
    }
    int err = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len)) {
        return -1;
    }
    if (!err) {
        return 0;
    } else {
        errno = err;
        return -1;
    }
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = doSocketIo(sockfd, accept_f, "accept", tigerkin::IOManager::Event::READ, addr, addrlen);
    if (fd >= 0) {
        tigerkin::SingletonFdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return doSocketIo(fd, read_f, "read", tigerkin::IOManager::Event::READ, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return doSocketIo(fd, readv_f, "readv", tigerkin::IOManager::READ, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return doSocketIo(sockfd, recv_f, "recv", tigerkin::IOManager::Event::READ, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return doSocketIo(sockfd, recvfrom_f, "recvfrom", tigerkin::IOManager::Event::READ, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return doSocketIo(sockfd, recvmsg_f, "recvmsg", tigerkin::IOManager::Event::READ, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return doSocketIo(fd, write_f, "write", tigerkin::IOManager::Event::WRITE, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return doSocketIo(fd, writev_f, "writev", tigerkin::IOManager::Event::WRITE, iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return doSocketIo(sockfd, send_f, "send", tigerkin::IOManager::Event::WRITE, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
    return doSocketIo(sockfd, sendto_f, "sendto", tigerkin::IOManager::Event::WRITE, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    return doSocketIo(sockfd, sendmsg_f, "sendmsg", tigerkin::IOManager::Event::WRITE, msg, flags);
}

int close(int fd) {
    if (!tigerkin::isEnableHook()) {
        return close_f(fd);
    }
    tigerkin::FdEntity::ptr fdEntity = tigerkin::SingletonFdMgr::GetInstance()->get(fd);
    if (fdEntity) {
        tigerkin::IOManager *iom = tigerkin::IOManager::GetThis();
        if (iom) {
            iom->cancelAllEvent(fd);
        }
        tigerkin::SingletonFdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */) {
    va_list ap;
    va_start(ap, cmd);
    if (!tigerkin::isEnableHook()) {
        va_end(ap);
        return fcntl_f(fd, cmd);
    }
    switch (cmd) {
        case F_SETFL: {
            int arg = va_arg(ap, int);
            va_end(ap);
            tigerkin::FdEntity::ptr fdEntity = tigerkin::SingletonFdMgr::GetInstance()->get(fd);
            if (!fdEntity || !fdEntity->isSocket() || !fdEntity->isClosed()) {
                return fcntl_f(fd, cmd, arg);
            }
            fdEntity->setUserNonblock(arg & O_NONBLOCK);
            if (fdEntity->isSysNonblock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= !O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
        case F_GETFL: {
            va_end(ap);
            int flag = fcntl_f(fd, cmd);
            tigerkin::FdEntity::ptr fdEntity = tigerkin::SingletonFdMgr::GetInstance()->get(fd);
            if (!fdEntity || !fdEntity->isSocket() || !fdEntity->isClosed()) {
                return flag;
            }
            if (fdEntity->isUserNonblock()) {
                return flag | O_NONBLOCK;
            } else {
                return flag &= ~O_NONBLOCK;
            }
            return flag;
        }

        case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
        case F_DUPFD_CLOEXEC:
#endif
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
#ifdef F_ADD_SEALS
        case F_ADD_SEALS:
#endif
        case F_NOTIFY: {
            int arg = va_arg(ap, int);
            va_end(ap);
            return fcntl_f(fd, cmd, arg);
        }

        case F_GETOWN:
        case F_GETSIG:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
#ifdef F_GET_SEALS
        case F_GET_SEALS:
#endif
        case F_GETLEASE: {
            va_end(ap);
            return fcntl_f(fd, cmd);
        }

        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
#ifdef F_OFD_SETLK
        case F_OFD_SETLK:
#endif
#ifdef F_OFD_SETLKW
        case F_OFD_SETLKW:
#endif
#ifdef F_OFD_GETLK
        case F_OFD_GETLK:
#endif
        {
            struct flock *arg = va_arg(ap, struct flock *);
            va_end(ap);
            return fcntl_f(fd, cmd, arg);
        }

        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            struct f_owner_ex *arg = va_arg(ap, struct f_owner_ex *);
            va_end(ap);
            return fcntl_f(fd, cmd, arg);
        }

#ifdef F_GET_RW_HINT
        case F_GET_RW_HINT:
#endif
#ifdef F_SET_RW_HINT
        case F_SET_RW_HINT:
#endif
#ifdef F_GET_FILE_RW_HINT
        case F_GET_FILE_RW_HINT:
#endif
#ifdef F_SET_FILE_RW_HINT
        case F_SET_FILE_RW_HINT:
#endif
        {
            uint64_t *arg = va_arg(ap, uint64_t *);
            va_end(ap);
            return fcntl_f(fd, cmd, arg);
        }

        default: {
            va_end(ap);
            return fcntl_f(fd, cmd);
        }
    }
}

int ioctl(int fd, unsigned long request, ...) {
    va_list ap;
    va_start(ap, request);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    if (FIONBIO == request) {
        bool block = !!*(int *)arg;
        tigerkin::FdEntity::ptr fdEntity = tigerkin::SingletonFdMgr::GetInstance()->get(fd);
        if (!fdEntity || !fdEntity->isSocket() || !fdEntity->isClosed()) {
            return ioctl_f(fd, request, arg);
        }
        fdEntity->setUserNonblock(block);
    }
    return ioctl_f(fd, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (!tigerkin::isEnableHook()) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if (level == SOL_SOCKET) {
        tigerkin::FdEntity::ptr fdEntity = tigerkin::SingletonFdMgr::GetInstance()->get(sockfd);
        if (!fdEntity || !fdEntity->isSocket() || !fdEntity->isClosed()) {
            return setsockopt_f(sockfd, level, optname, optval, optlen);
        }
        const timeval *v = (const timeval *)optval;
        if (optname == SO_RCVTIMEO) {
            fdEntity->setRecvTimeout(v->tv_sec * 1000 + v->tv_usec);
        } else if (optname == SO_SNDTIMEO) {
            fdEntity->setSendTimeout(v->tv_sec * 1000 + v->tv_usec);
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}
