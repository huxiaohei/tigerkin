/*****************************************************************
 * Description 调度器
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/20
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "scheduler.h"

#include "hook.h"

namespace tigerkin {

static thread_local Scheduler *t_scheduler = nullptr;

Scheduler::Scheduler(size_t threads, bool useCaller, const std::string &name)
    : m_name(name), m_userCaller(useCaller) {
    TIGERKIN_ASSERT2(threads > 0, "One scheduler must have at least one thread");
    if (useCaller) {
        --threads;
        TIGERKIN_ASSERT2(GetThis() == nullptr, "A thread can only have one scheduler");
        Thread::SetName(name + "0");
        t_scheduler = this;
        m_callerCo.reset(new Coroutine(std::bind(&Scheduler::run, this)));
        Coroutine::SetCallerCo(m_callerCo);
        m_callerThreadId = GetThreadId();
        m_threadIds.push_back(m_callerThreadId);
    }
    m_threadCnt = threads;
}

Scheduler::~Scheduler() {
    TIGERKIN_ASSERT(m_stopping);
    if (GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler *Scheduler::GetThis() {
    return t_scheduler;
}

void Scheduler::start() {
    Mutex::Lock lock(m_mutex);
    if (!m_stopping) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "The scheduler can not restart while it is stopping"
                                                      << BacktraceToString();
        return;
    }
    m_stopping = false;
    m_autoStop = false;
    TIGERKIN_ASSERT(m_threads.empty());
    m_threads.resize(m_threadCnt);
    for (size_t i = 0; i < m_threadCnt; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + std::to_string(i + 1)));
        m_threadIds.push_back(m_threads[i]->getId());
        ++m_activeThreadCnt;
    }
    lock.unlock();
    if (m_userCaller && m_callerCo->getState() == Coroutine::State::INIT) {
        m_callerCo->resume();
    }
}

void Scheduler::stop() {
    m_autoStop = true;
    if (m_userCaller && m_threadCnt == 0 && (m_callerCo->getState() == Coroutine::TERMINAL || m_callerCo->getState() == Coroutine::INIT)) {
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "STOP";
        m_stopping = true;
        if (stopping()) {
            return;
        }
    }
    m_stopping = true;
    if (!m_userCaller) {
        for (size_t i = 0; i < m_threadCnt; ++i) {
            m_threads[i]->join();
        }
    }
}

void Scheduler::run() {
    setEnableHook(true);
    setThis();
    Coroutine::ptr idleCo(new Coroutine(std::bind(&Scheduler::idle, this)));
    Task task;
    while (true) {
        task.reset();
        bool needTickle = false;
        bool isActive = false;
        {
            Mutex::Lock lock(m_mutex);
            std::list<Task>::iterator it = m_taskPools.begin();
            while (it != m_taskPools.end()) {
                if (it->threadId != 0 && it->threadId != GetThreadId()) {
                    ++it;
                    needTickle = true;
                    continue;
                }
                TIGERKIN_ASSERT(it->co || it->cb);
                task = *it;
                m_taskPools.erase(it);
                isActive = true;
                break;
            }
            if (!m_taskPools.empty() && m_idleThreadCnt > 0) {
                needTickle = true;
            }
        }
        if (needTickle) {
            tickle();
        }
        if (task.co && (task.co->getState() != Coroutine::State::TERMINAL &&
                        task.co->getState() != Coroutine::State::EXCEPT)) {
            if (task.co->getStackId() > 0) {
                task.co->resume();
            } else {
                task.co->resumeWithNewStack();
            }
            task.threadId = task.co->getThreadId();
            if (task.co->getState() == Coroutine::State::READY) {
                schedule(task.co, task.threadId, true);
            }
            task.reset();
        } else if (task.cb) {
            Coroutine::ptr co(new Coroutine(task.cb));
            schedule(co, task.threadId, true);
            task.reset();
        } else {
            if (isActive) {
                task.reset();
                continue;
            }
            if (idleCo->getState() == Coroutine::State::TERMINAL ||
                idleCo->getState() == Coroutine::State::EXCEPT) {
                TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE COROUTINE TERMINAL OR EXCEPT";
                break;
            }

            {   
                if (m_autoStop) {
                    tickle();
                }
                Mutex::Lock lock(m_mutex);
                if (m_callerThreadId == GetThreadId()) {
                    if (stopping()) {
                        Coroutine::DelCallerCo();
                        idleCo->resume();
                        break;
                    } else {
                        ++m_idleThreadCnt;
                        lock.unlock();
                        idleCo->resume();
                        --m_idleThreadCnt;
                    }
                } else if (canEarlyClosure()) {
                    --m_activeThreadCnt;
                    idleCo->resume();
                    tickle();
                    break;
                } else {
                    if (m_autoStop) {
                        tickle();
                        continue;
                    }
                    ++m_idleThreadCnt;
                    lock.unlock();
                    idleCo->resume();
                    --m_idleThreadCnt;
                }
            }
        }
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

bool Scheduler::stopping() {
    return m_autoStop && m_stopping && m_taskPools.empty() && m_activeThreadCnt == 0;
}

bool Scheduler::canEarlyClosure() {
    return m_autoStop && m_taskPools.empty() && m_idleThreadCnt == 0;
}

void Scheduler::tickle() {
    Thread::CondSignal(m_cond, m_threadMutex, m_idleThreadCnt);
}

void Scheduler::idle() {
    while (!stopping()) {
        if (GetThreadId() != m_callerThreadId && canEarlyClosure()) {
            break;
        }
        Thread::CondWait(m_cond, m_threadMutex);
        Coroutine::Yield();
    }
}

}  // namespace tigerkin
