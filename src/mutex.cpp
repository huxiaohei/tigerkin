/*****************************************************************
 * Description mutex
 * Email huxiaoheigame@gmail.com
 * Created on 2021/09/12
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "mutex.h"

#include "macro.h"

namespace tigerkin {

Semaphore::Semaphore(uint32_t count) {
    if (sem_init(&m_semaphore, 0, count)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "sem_init error";
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore() {
    sem_destroy(&m_semaphore);
}

void Semaphore::wait() {
    if (sem_wait(&m_semaphore)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "sem_wait error";
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::semPost() {
    if (sem_post(&m_semaphore)) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "sem_post error";
        throw std::logic_error("sem_post error");
    }
}

}  // namespace tigerkin