/*****************************************************************
 * Description 调度器
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/20
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <atomic>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "coroutine.h"
#include "macro.h"
#include "mutex.h"
#include "thread.h"

namespace tigerkin {

class Scheduler : public std::enable_shared_from_this<Scheduler> {
   public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef MutexLock Mutex;

    Scheduler(size_t threadCnt = 1, bool useCaller = true, const std::string &name = "");
    virtual ~Scheduler();

    const std::string &getName() const { return m_name; }
    const uint8_t getSchedulerTickleCaller() const;
    void start();
    void stop();

    template <class OnceTask>
    void schedule(OnceTask t, uint64_t threadId = 0, bool force = false) {
        bool needTickle = false;
        {
            Mutex::Lock lock(m_mutex);
            needTickle = scheduleWithoutLock(t, threadId, force);
        }
        if (needTickle) {
            tickle();
        }
    }

    template <class OnceTaskIterator>
    void scheduleIterator(OnceTaskIterator begin, OnceTaskIterator end, bool force = false) {
        bool needTickle = false;
        {
            MutexLock::Lock lock(m_mutex);
            while (begin != end) {
                needTickle = scheduleWithoutLock(&*begin, 0, force) ? true : needTickle;
                ++begin;
            }
        }
        if (needTickle) {
            tickle();
        }
    }

   public:
    static Scheduler *GetThis();

   private:
    struct Task {
        Coroutine::ptr co;
        std::function<void()> cb;
        pid_t threadId;

        Task(Coroutine::ptr c, pid_t id = 0)
            : co(c), threadId(id) {
        }

        Task(Coroutine::ptr *c, pid_t id = 0)
            : threadId(id) {
            co.swap(*c);
        }

        Task(std::function<void()> f, pid_t id = 0)
            : cb(f), threadId(id) {
        }

        Task(std::function<void()> *f, pid_t id = 0)
            : threadId(id) {
            cb.swap(*f);
        }

        Task() {
            reset();
        }

        void reset() {
            threadId = 0;
            co = nullptr;
            cb = nullptr;
        }
    };

   private:
    template <class OnceTask>
    bool scheduleWithoutLock(OnceTask t, pid_t threadId = 0, bool force = false) {
        if (m_autoStop && !force) {
            TIGERKIN_LOG_WARN(TIGERKIN_LOG_NAME(SYSTEM)) << "CAN NOT ADD TASK WHILE THE SCHEDULER IS STOPPING\n\t"
                                                         << BacktraceToString();
            return false;
        }
        Task task(t, threadId);
        if (task.cb || task.co) {
            m_taskPools.push_back(task);
        }
        if (task.co && task.co->getThreadId() > 0) {
            task.threadId = task.co->getThreadId();
        }
        return !m_taskPools.empty() && m_idleThreadCnt > 0;
    }

   private:
    std::vector<pid_t> m_threadIds;
    std::atomic<size_t> m_activeThreadCnt = {0};
    std::atomic<size_t> m_idleThreadCnt = {0};
    bool m_stopping = true;
    bool m_autoStop = false;
    Mutex m_mutex;
    Thread::ThreadCond m_cond = PTHREAD_COND_INITIALIZER;
    Thread::ThreadCond m_callerCond = PTHREAD_COND_INITIALIZER;
    Thread::ThreadMutex m_threadMutex = PTHREAD_MUTEX_INITIALIZER;
    std::vector<Thread::ptr> m_threads;
    std::string m_name;

   protected:
    bool m_userCaller;
    pid_t m_callerThreadId = 0;
    Coroutine::ptr m_callerCo = nullptr;
    std::atomic<size_t> m_threadCnt = {0};
    std::list<Task> m_taskPools;
    bool hasIdelThreads() { return m_idleThreadCnt > 0; };
    void setThis();
    void run();
    virtual bool stopping();
    virtual bool canEarlyClosure();
    virtual void tickle();
    virtual void idle();
};

}  // namespace tigerkin

#endif