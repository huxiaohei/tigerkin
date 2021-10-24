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
static thread_local Coroutine::ptr t_runingCo = nullptr;

static ConfigVar<uint8_t>::ptr g_scheduler_tickle_caller = Config::Lookup<uint8_t>("tigerkin.scheduler.tickleCaller", 3, "TickleCaller");

Scheduler::Scheduler(size_t threads, bool useCaller, const std::string &name)
    : m_name(name) {
    TIGERKIN_ASSERT2(threads > 0, "One scheduler must have at least one thread");
    if (useCaller) {
        --threads;
        TIGERKIN_ASSERT2(GetThis() == nullptr, "A thread can only have one scheduler");
        Thread::SetName("Scheduler_" + name + "_0");
        t_scheduler = this;
        m_callerCo.reset(new Coroutine(std::bind(&Scheduler::run, this)));
        t_runingCo = m_callerCo;
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

const uint8_t Scheduler::getSchedulerTickleCaller() const {
    return g_scheduler_tickle_caller->getValue();
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
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), "Scheduler_" + m_name + "_" + std::to_string(i + 1)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
    if (t_runingCo) {
        t_runingCo->resume();
        t_runingCo = Coroutine::GetThis();
    }
}

void Scheduler::stop() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "STOP";
    m_stopping = true;
    m_autoStop = true;
    tickle(true, true);
    for (size_t i = 0; i < m_threads.size(); ++i) {
        m_threads[i]->join();
    }
}

void Scheduler::run() {
    setEnableHook(true);
    setThis();
    t_runingCo = Coroutine::GetThis();
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
                ++m_activeThreadCnt;
                break;
            }
            if (!m_taskPools.empty() && m_idleThreadCnt > 0) {
                needTickle = true;
            }
        }
        if (needTickle) {
            tickle(false);
        }
        if (task.co && (task.co->getState() != Coroutine::State::TERMINAL &&
                        task.co->getState() != Coroutine::State::EXCEPT)) {
            task.co->resume();
            --m_activeThreadCnt;
            if (task.co->getState() == Coroutine::State::READY) {
                schedule(task.co, task.threadId);
            }
            task.reset();
        } else if (task.cb) {
            Coroutine::ptr co(new Coroutine(task.cb));
            schedule(co, task.threadId);
            --m_activeThreadCnt;
            task.reset();
        } else {
            if (isActive) {
                --m_activeThreadCnt;
                task.reset();
                continue;
            }
            if (idleCo->getState() == Coroutine::State::TERMINAL ||
                idleCo->getState() == Coroutine::State::EXCEPT) {
                TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE COROUTINE TERMINAL OR EXCEPT";
                break;
            }
            if (m_callerThreadId == GetThreadId()) {
                if (!stopping())
                    Coroutine::Yield();
                else
                    break;
            } else if (m_autoStop) {
                break;
            } else {
                ++m_idleThreadCnt;
                idleCo->resume();
                --m_idleThreadCnt;
            }
        }
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

bool Scheduler::stopping() {
    Mutex::Lock lock(m_mutex);
    return m_autoStop && m_stopping && m_taskPools.empty() && m_activeThreadCnt == 0;
}

void Scheduler::tickle(bool tickleCaller, bool force) {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "TICKER";
    Thread::CondSignal(m_cond, m_threadMutex, m_idleThreadCnt);
    if (tickleCaller && m_callerThreadId == GetThreadId() && m_callerCo->getState() == Coroutine::State::YIELD) {
        if (force || m_taskPools.size() >= g_scheduler_tickle_caller->getValue() * m_threadCnt) {
            Coroutine::Resume(m_callerCo->getStackId());
        }
    }
}

void Scheduler::idle() {
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE START";
    while (!stopping()) {
        Thread::CondWait(m_cond, m_threadMutex);
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE END";
        Coroutine::Yield();
        TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE START";
    }
    TIGERKIN_LOG_INFO(TIGERKIN_LOG_NAME(SYSTEM)) << "IDLE END";
}

}  // namespace tigerkin
