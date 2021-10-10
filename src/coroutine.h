/*****************************************************************
* Description coroutine
* Email huxiaoheigame@gmail.com
* Created on 2021/09/19
* Copyright (c) 2021 虎小黑
****************************************************************/

#ifndef __TIGERKIN_COROUTINE_H__
#define __TIGERKIN_COROUTINE_H__

#include <ucontext.h>

#include <functional>
#include <memory>

#include "thread.h"

namespace tigerkin {

class Coroutine : public std::enable_shared_from_this<Coroutine> {
   public:
    typedef std::shared_ptr<Coroutine> ptr;
    enum State {
        INIT,
        YIELD,
        READY,
        EXECING,
        TERMINAL,
        EXCEPT
    };

    Coroutine(std::function<void()> cb, size_t stacksize = 0);
    ~Coroutine();

    /**
     * The coroutine's state must be INIT or TERMINAL
     */
    void reset(std::function<void()> cb);
    /**
     * From the current coroutine switch to another coroutine
     */
    void resume();
    /**
     * From the current coroutine switch to the current thread's main coroutine
     */
    void yield();
    /**
     * Get the coroutine id
     */
    uint64_t getId() const { return m_id; }
    /**
     * Get the coroutine stack id
     */
    uint64_t getStackId() const { return m_stackId; }
    /**
     * Get the coroutine state
     */
    State getState() const { return m_state; }
    
   public:
    static void SetThis(Coroutine *co);
    static Coroutine::ptr GetThis();
    static uint64_t GetCoId();
    static void Yield();
    static void Resume(uint64_t stackId);
    static size_t CoCnt();
    static size_t CoStackCnt();
    static void MainFunc();
    

   private:
    Coroutine();
    uint64_t m_id = 0;
    uint64_t m_stackId = 0;
    uint32_t m_stackSize = 0;
    State m_state = State::INIT;
    ucontext_t m_ctx;
    void *m_stack = nullptr;
    std::function<void()> m_cb;
};

}  // namespace tigerkin

#endif