/*****************************************************************
* Description 调度器
* Email huxiaoheigame@gmail.com
* Created on 2021/09/20
* Copyright (c) 2021 虎小黑
****************************************************************/

#include "scheduler.h"

namespace tigerkin {

static thread_local Scheduler *t_scheduler = nullptr;
static thread_local Coroutine::ptr t_runingCo = nullptr;

Scheduler::Scheduler(size_t threads, bool useCaller, const std::string &name)
    : m_name(name) {
    TIGERKIN_ASSERT2(threads > 0, "One scheduler must have at least one thread");
    if (useCaller) {
        --threads;
        TIGERKIN_ASSERT2(GetThis() == nullptr, "A thread can only have one scheduler");
        Thread::SetName("Scheduler_" + name + "_0");
        t_scheduler = this;
        t_runingCo.reset(new Coroutine(std::bind(&Scheduler::run, this)));
        m_threadIds.push_back(GetThreadId());
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
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "The scheduler can not restart while it is stopping";
        return;
    }
    m_stopping = false;
    m_autoStop = false;
    TIGERKIN_ASSERT(m_threads.empty());
    m_threads.resize(m_threadCnt);
    for (size_t i = 0; i < m_threadCnt; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), "Scheduler_" + m_name + std::to_string(i + 1)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
    if (t_runingCo) {
        t_runingCo->resume();
        t_runingCo = Coroutine::GetThis();
    }
}

void Scheduler::stop() {
    // if (m_rootThreadId != 0) {
    //     TIGERKIN_ASSERT2(GetThis() == this, "In the case of using `userCaller = true`, the scheduler can only be stopped in the thread that created this scheduler");
    // } else {
    //     TIGERKIN_ASSERT2(GetThis() != this, "In the case of using `userCaller = false`, the scheduler cannot be stopped in the thread that created this scheduler");
    // }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "STOP";
    m_stopping = true;
    m_autoStop = true;
    for (size_t i = 0; i < m_threadCnt; ++i) {
        tickle();
    }
    if (stopping()) {
        return;
    }
}

void Scheduler::run() {
    setThis();
    t_runingCo = Coroutine::GetThis();
    Coroutine::ptr idleCo(new Coroutine(std::bind(&Scheduler::idle, this)));
    Coroutine::ptr cbCo;
    Task task;
    while (true) {
        task.reset();
        bool needTickle = false;
        {
            Mutex::Lock lock(m_mutex);
            auto it = m_taskPools.begin();
            while (it != m_taskPools.end()) {
                if (it->threadId != 0 && it->threadId != GetThreadId()) {
                    ++it;
                    needTickle = true;
                    continue;
                }
                TIGERKIN_ASSERT(it->co || it->cb);
                task = *it;
                m_taskPools.erase(it);
                break;
            }
        }
        if (needTickle) {
            tickle();
        }
        if (task.co && (task.co->getState() != Coroutine::State::TERMINAL &&
                        task.co->getState() != Coroutine::State::EXCEPT)) {
            ++m_activeThreadCnt;
            task.co->resume();
            --m_activeThreadCnt;
            if (task.co->getState() == Coroutine::State::READY) {
                schedule(task.co, task.threadId);
            }
        } else if (task.cb) {
            if (cbCo) {
                cbCo->reset(task.cb);
            } else {
                cbCo.reset(new Coroutine(task.cb));
            }
            schedule(cbCo, task.threadId);
        } else {
            if (idleCo->getState() == Coroutine::State::TERMINAL ||
                idleCo->getState() == Coroutine::State::EXCEPT) {
                TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE COROUTINE TERMINAL OR EXCEPT";
                break;
            }
            ++m_idleThreadCnt;
            idleCo->resume();
            --m_idleThreadCnt;
        }
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

bool Scheduler::stopping() {
    Mutex::Lock lock(m_mutex);
    return m_stopping && m_taskPools.empty() && m_activeThreadCnt == 0;
}

void Scheduler::tickle() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "TICKER";
}

void Scheduler::idle() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE";
}

}  // namespace tigerkin
