/*****************************************************************
 * Description thread
 * Email huxiaoheigame@gmail.com
 * Created on 2021/08/21
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "thread.h"

#include "macro.h"
#include "util.h"

namespace tigerkin {

static thread_local Thread *cur_thread = nullptr;
static thread_local std::string cur_thread_name = "UNKNOW";

Thread *Thread::GetThis() {
    return cur_thread;
}

const std::string &Thread::GetName() {
    return cur_thread_name;
}

void Thread::SetName(const std::string &name) {
    if (name.empty()) {
        return;
    }
    if (cur_thread) {
        cur_thread->m_name = name;
    }
    cur_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string &name)
    : m_cb(cb), m_name(name) {
    if (name.empty()) {
        m_name = "UNKNOW";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::Run, this);
    if (rt) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "pthread_create fail\n"
                                                      << "\t rt:" << rt << "\n"
                                                      << "\t name:" << name;
        throw std::logic_error("pthread_create error");
    }
    // Waiting a semaphore is to ensure the thread is running or executed when the constructor function is executed
    //  1. in the thread of callback function, the sem_post function will be called to increase semaphore's value
    //  2. once semaphore's value is above zero, the waiting will exit
    //  3. so the constructor function is already exited, the thread is running or executed
    m_semaphore.wait();
}

Thread::~Thread() {
    if (m_thread) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if (m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if (rt) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "pthread_join fail\n"
                                                          << "\t rt:" << rt << "\n"
                                                          << "\t name:" << m_name;
            throw std::logic_error("pthread_join error");
        }
    }
    m_thread = 0;
}

void *Thread::Run(void *arg) {
    // has enter a thread instance
    try {
        Thread *thread = (Thread *)arg;
        cur_thread = thread;
        cur_thread_name = thread->m_name;
        thread->m_id = tigerkin::GetThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        std::function<void()> cb;
        cb.swap(thread->m_cb);
        thread->m_semaphore.semPost();
        cb();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}

}  // namespace tigerkin
