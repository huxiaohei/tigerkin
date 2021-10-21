/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2021/10/20
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_TIMER_H__
#define __TIGERKIN_TIMER_H__

#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "mutex.h"

namespace tigerkin {

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {
   public:
    friend TimerManager;
    typedef std::shared_ptr<Timer> ptr;
    Timer(uint64_t ms, std::function<void()> cb, TimerManager *mgr, bool recurring);

    bool cancel();
    bool refresh();
    bool reset(uint64_t ms, std::function<void()> cb = nullptr);

   private:
    Timer(uint64_t next);
    struct Comparator {
        bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const;
    };

   private:
        uint64_t m_ms = 0;
    uint64_t m_next = 0;
    std::function<void()> m_cb = nullptr;
    bool m_recurring = false;
    TimerManager *m_mgr = nullptr;
};

class TimerManager {
   public:
    friend Timer;
    typedef ReadWriteLock RWMutex;
    TimerManager();
    virtual ~TimerManager();
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
    Timer::ptr addCondTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> cond, bool recurring = false);
    void cancelAllTimers();
    void listExpiredCbs(std::vector<std::function<void()>> &cbs);
    uint64_t getNextTime();

   protected:
    virtual void onTimerRefresh() = 0;

   private:
    RWMutex m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
};

}  // namespace tigerkin

#endif
