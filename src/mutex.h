/*****************************************************************
 * Description mutex
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#ifndef __TIGERKIN__MUTEX_H__
#define __TIGERKIN__MUTEX_H__

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

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

template <class T>
class ScopedLockTmpl {
   public:
    ScopedLockTmpl(T &mutex)
        : m_mutex(mutex), m_locked(false) {
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

class SpinLock {
   public:
    typedef ScopedLockTmpl<SpinLock> Lock;

    SpinLock() {
        pthread_spin_init(&m_lock, PTHREAD_PROCESS_SHARED);
    }

    ~SpinLock() {
        pthread_spin_destroy(&m_lock);
    }

    void lock() {
        pthread_spin_lock(&m_lock);
    }

    void unlock() {
        pthread_spin_unlock(&m_lock);
    }

   private:
    pthread_spinlock_t m_lock;
};

class MutexLock {
   public:
    typedef ScopedLockTmpl<MutexLock> Lock;

    MutexLock() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~MutexLock() {
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

template <class T>
class ReadScopedLockImpl {
   public:
    ReadScopedLockImpl(T &mutex)
        : m_mutex(mutex), m_locked(false) {
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

template <class T>
class WriteScopedLockImpl {
   public:
    WriteScopedLockImpl(T &mutex)
        : m_mutex(mutex), m_locked(false) {
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

class ReadWriteLock {
   public:
    typedef ReadScopedLockImpl<ReadWriteLock> ReadLock;
    typedef WriteScopedLockImpl<ReadWriteLock> WriteLock;

    ReadWriteLock() {
        pthread_rwlock_init(&m_mutex, nullptr);
    }

    ~ReadWriteLock() {
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

}  // namespace tigerkin

#endif  // !__TIGERKIN__MUTEX_H__