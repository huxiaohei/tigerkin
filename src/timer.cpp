/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/20
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "timer.h"

#include "util.h"

namespace tigerkin {

bool Timer::Comparator::operator()(const Timer::ptr &lth, const Timer::ptr &rth) const {
    if (!lth && !rth) {
        return false;
    }
    if (!rth) {
        return true;
    }
    if (!lth) {
        return false;
    }
    if (lth->m_next <= rth->m_next) {
        return true;
    }
    return false;
}

Timer::Timer(uint64_t ms, std::function<void()> cb, TimerManager *mgr, bool recurring = false)
    : m_ms(ms), m_cb(cb), m_recurring(recurring), m_mgr(mgr) {
    m_next = GetNowMillisecond() + m_ms;
}

Timer::Timer(uint64_t next)
    : m_next(next) {
}

bool Timer::cancel() {
    TimerManager::RWMutex::WriteLock lock(m_mgr->m_mutex);
    if (m_cb) {
        m_cb = nullptr;
        std::set<Timer::ptr>::iterator it = m_mgr->m_timers.find(shared_from_this());
        if (it != m_mgr->m_timers.end()) {
            m_mgr->m_timers.erase(it);
            return true;
        }
        return false;
    }
    return false;
}

bool Timer::refresh() {
    TimerManager::RWMutex::WriteLock lock(m_mgr->m_mutex);
    if (!m_cb) {
        return false;
    }
    std::set<Timer::ptr>::iterator it = m_mgr->m_timers.find(shared_from_this());
    if (it == m_mgr->m_timers.end()) {
        m_cb = nullptr;
        return false;
    }
    m_mgr->m_timers.erase(it);
    m_next = GetNowMillisecond() + m_ms;
    it = m_mgr->m_timers.insert(shared_from_this()).first;
    if (it == m_mgr->m_timers.begin()) {
        m_mgr->onTimerRefresh();
    }
    return true;
}

bool Timer::reset(uint64_t ms, std::function<void()> cb) {
    TimerManager::RWMutex::WriteLock lock(m_mgr->m_mutex);
    if (!m_cb) {
        cb = nullptr;
        return false;
    }
    if (cb) {
        m_cb = nullptr;
        m_cb = cb;
    }
    m_ms = ms;
    m_next = m_ms + GetNowMillisecond();
    std::set<Timer::ptr>::iterator it = m_mgr->m_timers.find(shared_from_this());
    if (it == m_mgr->m_timers.end()) {
        m_cb = nullptr;
        return false;
    }
    m_mgr->m_timers.erase(it);
    m_next = GetNowMillisecond() + m_ms;
    it = m_mgr->m_timers.insert(shared_from_this()).first;
    if (it == m_mgr->m_timers.begin()) {
        m_mgr->onTimerRefresh();
    }
    return true;
}

TimerManager::TimerManager() {
}

TimerManager::~TimerManager() {
}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
    Timer::ptr timer(new Timer(ms, cb, this, recurring));
    RWMutex::WriteLock lock(m_mutex);
    std::set<Timer::ptr>::iterator it = m_timers.insert(timer).first;
    bool isFirst = it == m_timers.begin();
    lock.unlock();
    if (isFirst) {
        onTimerRefresh();
    }
    return *it;
}

static void condTimerCallback(std::function<void()> cb, std::weak_ptr<void> cond) {
    std::shared_ptr<void> tmp = cond.lock();
    if (tmp) {
        cb();
    }
}

Timer::ptr TimerManager::addCondTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> cond, bool recurring) {
    return addTimer(ms, std::bind(&condTimerCallback, cb, cond));
}

void TimerManager::cancelAllTimers() {
    RWMutex::WriteLock lock(m_mutex);
    while (m_timers.begin() != m_timers.end()) {
        (*m_timers.begin())->m_cb = nullptr;
        m_timers.erase(m_timers.begin());
    }
}

void TimerManager::listExpiredCbs(std::vector<std::function<void()>> &cbs) {
    uint64_t nowMs = GetNowMillisecond();
    {
        RWMutex::ReadLock lock(m_mutex);
        if (m_timers.empty()) {
            return;
        }
    }
    RWMutex::WriteLock lock(m_mutex);
    Timer::ptr nowTimer(new Timer(nowMs));
    std::set<Timer::ptr>::iterator it = m_timers.lower_bound(nowTimer);
    while (it != m_timers.end() && (*it)->m_next == nowMs) {
        ++it;
    }
    std::vector<Timer::ptr> expired;
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());
    for (Timer::ptr &timer : expired) {
        cbs.push_back(timer->m_cb);
        if (timer->m_recurring) {
            timer->m_next = nowMs + timer->m_ms;
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;
        }
    }
}

uint64_t TimerManager::getNextTime() {
    RWMutex::ReadLock lock(m_mutex);
    if (m_timers.empty()) {
        return ~0ull;
    }
    const Timer::ptr &next = *m_timers.begin();
    uint64_t nowMs = GetNowMillisecond();
    if (nowMs >= next->m_next) {
        return 0;
    } else {
        return next->m_next - nowMs;
    }
}

}  // namespace tigerkin