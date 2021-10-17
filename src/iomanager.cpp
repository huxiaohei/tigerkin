/*****************************************************************
* Description IO管理器(基于epoll)
* Email huxiaoheigame@gmail.com
* Created on 2021/10/13
* Copyright (c) 2021 虎小黑
****************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "iomamager.h"
#include "macro.h"

namespace tigerkin {

static ConfigVar<uint8_t>::ptr g_iomanager_tickle_caller = Config::Lookup<uint8_t>("tigerkin.iomanager.tickleCaller", 3, "TickleCaller");

IOManager::FdContext::EventContext &IOManager::FdContext::getEventContext(const IOManager::Event event) {
    if (event == IOManager::Event::READ) {
        return read;
    } else {
        return write;
    }
}

void IOManager::FdContext::resetContext(IOManager::FdContext::EventContext &ctx) {
    ctx.scheduler = nullptr;
    ctx.co.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(const IOManager::Event event) {
    TIGERKIN_ASSERT(events & event);
    events = (Event)(events & ~events);
    EventContext &ctx = getEventContext(event);
    if (ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.co);
    }
    ctx.scheduler = nullptr;
}

IOManager::IOManager(size_t threadCnt, bool useCaller, const std::string &name)
    : Scheduler(threadCnt, useCaller, name) {
    m_epfd = epoll_create(1000);
    TIGERKIN_ASSERT2(m_epfd > 0, "EPOLL CREATE ERROR");
    int rt = pipe(m_tickleFds);
    TIGERKIN_ASSERT(!rt);
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];
    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    TIGERKIN_ASSERT(!rt);
    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    TIGERKIN_ASSERT(!rt);
    fdContextsResize(64);
    start();
}

IOManager::~IOManager() {
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);
    for (size_t i = 0; i < m_fdContexts.size(); ++i) {
        if (m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

IOManager::IOManagerCode IOManager::addEvent(FdId fd, Event event, std::function<void()> cb) {
    FdContext *fdCtx = nullptr;
    RWMutex::ReadLock rLock(m_mutex);
    if ((int)m_fdContexts.size() > fd) {
        fdCtx = m_fdContexts[fd];
        rLock.unlock();
    } else {
        rLock.unlock();
        RWMutex::WriteLock wLock(m_mutex);
        fdContextsResize(fd * 1.5);
        fdCtx = m_fdContexts[fd];
    }
    FdContext::Mutex::Lock fdLock(fdCtx->mutex);
    if (fdCtx->events & event) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "add event fail:\n\t"
                                                      << " event=" << event << "\n\t"
                                                      << " fdCtx.events=" << fdCtx->events;
        return IOManagerCode::ADD_EVENT_ERR;
    }
    int op = fdCtx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epEvent;
    epEvent.events = EPOLLET | fdCtx->events | event;
    epEvent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, op, fd, &epEvent);
    if (rt) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "epoll ctl fail:\n\t"
                                                      << "op=" << op << "\n\t"
                                                      << "fd=" << fd << "\n\t"
                                                      << "epEvent.events=" << epEvent.events << "\n\t"
                                                      << "rt=" << rt << "\n\t"
                                                      << "errno=" << errno << "\n\t"
                                                      << "\t" << strerror(errno);
        return IOManagerCode::ADD_EVENT_ERR;
    }
    ++m_pendingEventCount;
    fdCtx->events = (Event)(fdCtx->events | event);
    FdContext::EventContext &eventCtx = fdCtx->getEventContext(event);
    TIGERKIN_ASSERT(!eventCtx.scheduler && !eventCtx.co && !eventCtx.cb);
    eventCtx.scheduler = Scheduler::GetThis();
    if (cb) {
        eventCtx.cb.swap(cb);
    } else {
        eventCtx.co = Coroutine::GetThis();
        TIGERKIN_ASSERT(eventCtx.co->getState() == Coroutine::State::EXECING);
    }
    return IOManagerCode::ADD_EVENT_SUC;
}

bool IOManager::delEvent(FdId fd, Event event) {
    RWMutex::ReadLock rLock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext *fdCtx = m_fdContexts[fd];
    rLock.unlock();
    FdContext::Mutex::Lock fdLock(fdCtx->mutex);
    if (!(fdCtx->events & event)) {
        return false;
    }
    Event newEvent = (Event)(fdCtx->events & ~event);
    int op = newEvent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epEvent;
    epEvent.events = EPOLLET | newEvent;
    epEvent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, op, fd, &epEvent);
    if (rt) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "epoll ctl fail:\n\t"
                                                      << "op=" << op << "\n\t"
                                                      << "fd=" << fd << "\n\t"
                                                      << "epEvent.events=" << epEvent.events << "\n\t"
                                                      << "rt=" << rt << "\n\t"
                                                      << "errno=" << errno << "\n\t"
                                                      << "\t" << strerror(errno);
        return false;
    }
    --m_pendingEventCount;
    fdCtx->events = newEvent;
    FdContext::EventContext &eventCtx = fdCtx->getEventContext(event);
    fdCtx->resetContext(eventCtx);
    return true;
}

bool IOManager::cancelEvent(FdId fd, Event event) {
    RWMutex::ReadLock rLock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext *fdCtx = m_fdContexts[fd];
    rLock.unlock();
    FdContext::Mutex::Lock fdLock(fdCtx->mutex);
    if (!(fdCtx->events & event)) {
        return false;
    }
    Event newEvent = (Event)(fdCtx->events & ~event);
    int op = newEvent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epEvent;
    epEvent.events = EPOLLET | newEvent;
    epEvent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, op, fd, &epEvent);
    if (rt) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "epoll ctl fail:\n\t"
                                                      << "op=" << op << "\n\t"
                                                      << "fd=" << fd << "\n\t"
                                                      << "epEvent.events=" << epEvent.events << "\n\t"
                                                      << "rt=" << rt << "\n\t"
                                                      << "errno=" << errno << "\n\t"
                                                      << "\t" << strerror(errno);
        return false;
    }
    --m_pendingEventCount;
    fdCtx->events = newEvent;
    fdCtx->triggerEvent(event);
    FdContext::EventContext &eventCtx = fdCtx->getEventContext(event);
    fdCtx->resetContext(eventCtx);
    return true;
}

bool IOManager::cancelAllEvent(FdId fd) {
    RWMutex::ReadLock rLock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext *fdCtx = m_fdContexts[fd];
    rLock.unlock();
    FdContext::Mutex::Lock fdLock(fdCtx->mutex);
    epoll_event epEvent;
    epEvent.events = 0;
    epEvent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &epEvent);
    if (rt) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "epoll ctl fail:\n\t"
                                                      << "op=" << EPOLL_CTL_DEL << "\n\t"
                                                      << "fd=" << fd << "\n\t"
                                                      << "epEvent.events=" << epEvent.events << "\n\t"
                                                      << "rt=" << rt << "\n\t"
                                                      << "errno=" << errno << "\n\t"
                                                      << "\t" << strerror(errno);
        return false;
    }
    if (fdCtx->events & Event::READ) {
        --m_pendingEventCount;
        fdCtx->triggerEvent(Event::READ);
        FdContext::EventContext &eventCtx = fdCtx->getEventContext(Event::READ);
        fdCtx->resetContext(eventCtx);
    }
    if (fdCtx->events & Event::WRITE) {
        --m_pendingEventCount;
        fdCtx->triggerEvent(Event::WRITE);
        FdContext::EventContext &eventCtx = fdCtx->getEventContext(Event::WRITE);
        fdCtx->resetContext(eventCtx);
    }
    TIGERKIN_ASSERT(fdCtx->events == 0);
    return true;
}

void IOManager::tickle(bool tickleCaller, bool force) {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "TICKER";
    if (hasIdelThreads()) {
        int rt = write(m_tickleFds[1], "T", 1);
        TIGERKIN_ASSERT(rt == 1);
    }
    if (tickleCaller && m_callerThreadId == GetThreadId() && m_callerCo->getState() == Coroutine::State::YIELD) {
        if (force || m_taskPools.size() >= g_iomanager_tickle_caller->getValue() * m_threadCnt) {
            Coroutine::Resume(m_callerCo->getStackId());
        }
    }
}

bool IOManager::stopping() {
    return Scheduler::stopping() && m_pendingEventCount == 0;
}

void IOManager::idle() {
    const int MAX_EPOLL_EVENT = 256;
    epoll_event *events = new epoll_event[MAX_EPOLL_EVENT]();
    std::shared_ptr<epoll_event> sharedEvents(events, [](epoll_event *ptr) {
        delete[] ptr;
    });
    int rt = 0;
    while (!stopping()) {
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE START";
        rt = 0;
        do {
            static const int EPOLL_MAX_TIMEOUT = 5000;
            rt = epoll_wait(m_epfd, events, MAX_EPOLL_EVENT, EPOLL_MAX_TIMEOUT);
            if (rt > 0) break;
        } while (!stopping());
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "has msg"
                                                     << " rt = " << rt;
        for (int i = 0; i < rt; ++i) {
            TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "has msg"
                                                         << " i = " << i;
            epoll_event &event = events[i];
            if (event.data.fd == m_tickleFds[0]) {
                uint8_t dummy;
                while(read(m_tickleFds[0], &dummy, 1) == 1);
                continue;
            }
            FdContext *fdCtx = (FdContext *)event.data.ptr;
            FdContext::Mutex::Lock lock(fdCtx->mutex);
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= EPOLLIN | EPOLLOUT;
            }
            int realEvent = IOManager::Event::NONE;
            if (event.events & EPOLLIN) {
                realEvent |= IOManager::Event::READ;
            }
            if (event.events & EPOLLOUT) {
                realEvent |= IOManager::Event::WRITE;
            }
            if ((fdCtx->events & realEvent) == IOManager::Event::NONE) {
                continue;
            }
            int leftEvents = (fdCtx->events & ~realEvent);
            int op = leftEvents ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | leftEvents;
            int rt2 = epoll_ctl(m_epfd, op, fdCtx->fd, &event);
            if (rt2) {
                TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "epoll ctl fail:\n\t"
                                                              << "op=" << op << "\n\t"
                                                              << "fd=" << fdCtx->fd << "\n\t"
                                                              << "epEvent.events=" << event.events << "\n\t"
                                                              << "rt=" << rt2 << "\n\t"
                                                              << "errno=" << errno << "\n\t"
                                                              << "\t" << strerror(errno);
                continue;
            }
            if (realEvent & IOManager::Event::READ) {
                fdCtx->triggerEvent(IOManager::Event::READ);
                --m_pendingEventCount;
            }
            if (realEvent & IOManager::Event::WRITE) {
                fdCtx->triggerEvent(IOManager::Event::WRITE);
                --m_pendingEventCount;
            }
        }
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE END";
        Coroutine::Yield();
    }
}

IOManager *IOManager::GetThis() {
    return dynamic_cast<IOManager *>(Scheduler::GetThis());
}

void IOManager::fdContextsResize(size_t size) {
    for (size_t i = size; i < m_fdContexts.size(); ++i) {
        if (m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
    m_fdContexts.resize(size);
    for (size_t i = 0; i < m_fdContexts.size(); ++i) {
        if (!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

}  // namespace tigerkin