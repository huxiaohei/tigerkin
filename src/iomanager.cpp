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

#include "hook.h"
#include "iomanager.h"
#include "macro.h"

namespace tigerkin {

IOManager::FdContext::EventContext &IOManager::FdContext::getEventContext(const IOManager::Event event) {
    if (event == IOManager::Event::READ) {
        return read;
    } else {
        return write;
    }
}

void IOManager::FdContext::resetContext(IOManager::FdContext::EventContext &ctx) {
    ctx.scheduler = nullptr;
    ctx.threadId = 0;
    ctx.co.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(const IOManager::Event event) {
    TIGERKIN_ASSERT(events & event);
    events = (Event)(events & ~event);
    EventContext &ctx = getEventContext(event);
    if (ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb, ctx.threadId, true);
    } else {
        ctx.scheduler->schedule(&ctx.co, ctx.threadId, true);
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
    rt = fcntl_f(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    TIGERKIN_ASSERT(!rt);
    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    TIGERKIN_ASSERT(!rt);
    fdContextsResize(64);
}

IOManager::~IOManager() {
    cancelAllTimers();
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
        eventCtx.threadId = GetThreadId();
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
    fdCtx->triggerEvent(event);
    --m_pendingEventCount;
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
    if (!fdCtx->events) {
        return false;
    }

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
        fdCtx->triggerEvent(Event::READ);
        --m_pendingEventCount;
    }
    if (fdCtx->events & Event::WRITE) {
        fdCtx->triggerEvent(Event::WRITE);
        --m_pendingEventCount;
    }
    TIGERKIN_ASSERT(fdCtx->events == IOManager::Event::NONE);
    return true;
}

void IOManager::tickle() {
    if (hasIdleThreads()) {
        int rt = write(m_tickleFds[1], "T", 1);
        TIGERKIN_ASSERT(rt == 1);
    }
}

bool IOManager::stopping() {
    return Scheduler::stopping() && m_pendingEventCount == 0;
}

bool IOManager::canEarlyClosure() {
    return Scheduler::canEarlyClosure() && m_pendingEventCount == 0;
}

void IOManager::idle() {
    const int MAX_EPOLL_EVENT = 256;
    epoll_event *events = new epoll_event[MAX_EPOLL_EVENT]();
    std::shared_ptr<epoll_event> sharedEvents(events, [](epoll_event *ptr) {
        delete[] ptr;
    });
    int rt = 0;
    std::vector<std::function<void()>> cbs;
    while (!stopping()) {
        if (GetThreadId() != m_callerThreadId && canEarlyClosure()) {
            break;
        }
        rt = 0;
        do {
            static const int EPOLL_MAX_TIMEOUT = 5000;
            uint64_t nextTime = getNextTime();
            rt = epoll_wait(m_epfd, events, MAX_EPOLL_EVENT, nextTime > EPOLL_MAX_TIMEOUT ? EPOLL_MAX_TIMEOUT : (int)nextTime);
            if (rt > 0 || getNextTime() <= 0 || isAutoStop()) break;
        } while (!stopping());

        listExpiredCbs(cbs);
        scheduleIterator(cbs.begin(), cbs.end());
        cbs.clear();

        for (int i = 0; i < rt; ++i) {
            epoll_event &event = events[i];
            if (event.data.fd == m_tickleFds[0]) {
                uint8_t dummy;
                while (read(m_tickleFds[0], &dummy, 1) == 1) {
                }
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
        Coroutine::Yield();
    }
}

void IOManager::onTimerRefresh() {
    tickle();
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

IOManager *IOManager::GetThis() {
    return dynamic_cast<IOManager *>(Scheduler::GetThis());
}

}  // namespace tigerkin