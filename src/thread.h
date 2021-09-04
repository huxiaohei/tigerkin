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

namespace tigerkin {

class Semaphore {
   public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void semPost();

   private:
    Semaphore(const Semaphore &) = delete;
    Semaphore(const Semaphore &&) = delete;
    Semaphore &operator=(const Semaphore &) = delete;

   private:
    sem_t m_semaphore;
};

template<class T>
class ScopedLockTmpl {
   public:
    ScopedLockTmpl(T &mutex)
        :m_mutex(mutex), m_locked(false) {
        lock();
    }

    ~ScopedLockTmpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

   private:
    T &m_mutex;
    bool m_locked;
};

template<class T>
class ReadScopedLockImpl {
   public:
    ReadScopedLockImpl(T &mutex)
        :m_mutex(mutex), m_locked(false) {
        lock();
    }

    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
   private:
    T &m_mutex; 
    bool m_locked;
};

class Mutex {
   public:
    typedef ScopedLockTmpl<Mutex> Lock;

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    
    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
   private:
    pthread_mutex_t m_mutex;
};

template<class T>
class WriteScopedLockImpl {
   public:
    WriteScopedLockImpl(T &mutex)
        :m_mutex(mutex), m_locked(false) {
        lock();
    }

    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
   private:
    T &m_mutex; 
    bool m_locked;
};

class ReadWriteMutex {

   public:
    typedef ReadScopedLockImpl<ReadWriteMutex> ReadMutex;
    typedef WriteScopedLockImpl<ReadWriteMutex> WriteMutex;

    ReadWriteMutex() {
        pthread_rwlock_init(&m_mutex, nullptr);
    }

    ~ReadWriteMutex() {
        pthread_rwlock_destroy(&m_mutex);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_mutex);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_mutex);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_mutex);
    }

   private:
    pthread_rwlock_t m_mutex;

};

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