/*****************************************************************
* Description 调度器
* Email huxiaoheigame@gmail.com
* Created on 2021/09/20
* Copyright (c) 2021 虎小黑
****************************************************************/

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <atomic>
#include <memory>
#include <string>
#include <vector>

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
    void start();
    void stop();

    template <class OnceTask>
    void schedule(OnceTask t, uint64_t threadId = 0) {
        bool needTickle = false;
        {
            Mutex::Lock lock(m_mutex);
            needTickle = scheduleWithoutLock(t, threadId);
        }
        if (needTickle) {
            tickle();
        }
    }

    template <class OnceTaskIterator>
    void scheduleIterator(OnceTaskIterator begin, OnceTaskIterator end) {
        bool needTickle = false;
        {
            MutexLock::Lock lock(m_mutex);
            while (begin != end) {
                needTickle = scheduleWithoutLock(&*begin, 0) ? true : needTickle;
                ++begin;
            }
        }
        if (needTickle) {
            tickle();
        }
    }

   public:
    static Scheduler *GetThis();

   protected:
    std::vector<uint64_t> m_threadIds;
    size_t m_threadCnt = 0;
    std::atomic<size_t> m_activeThreadCnt = {0};
    std::atomic<size_t> m_idleThreadCnt = {0};
    bool m_stopping = true;
    bool m_autoStop = false;

   protected:
    void setThis();
    void run();
    virtual bool stopping();
    virtual void tickle();
    virtual void idle();

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
    bool scheduleWithoutLock(OnceTask t, uint64_t threadId = 0) {
        if (m_autoStop) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "Can not add task while the scheduler is stopping";
            return false;
        }
        bool needTickle = m_taskPools.empty();
        Task task(t, threadId);
        if (task.co || task.cb) {
            m_taskPools.push_back(task);
        }
        return needTickle;
    }

   private:
    Mutex m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::vector<Task> m_taskPools;
    std::string m_name;
};

}  // namespace tigerkin

#endif