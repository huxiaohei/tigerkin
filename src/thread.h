/*****************************************************************
 * Description thread
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN_THREAD_H__
#define __TIGERKIN_THREAD_H__

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include <functional>
#include <memory>
#include <thread>

#include "mutex.h"

namespace tigerkin {

class Thread {
   public:
    typedef std::shared_ptr<Thread> ptr;

    static Thread *GetThis();
    static const std::string &GetName();
    static void SetName(const std::string &name);

    Thread(std::function<void()> cb, const std::string &name);
    ~Thread();
    void join();
    const pid_t getId() const { return m_id; }
    const std::string &getName() const { return m_name; }

   private:
    // forbid copy
    Thread(const Thread &) = delete;
    Thread(const Thread &&) = delete;
    Thread &operator=(const Thread &) = delete;

   private:
    pid_t m_id;
    pthread_t m_thread;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;

   private:
    static void *Run(void *arg);
};
}  // namespace tigerkin

#endif