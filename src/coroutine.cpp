/*****************************************************************
* Description coroutine
* Email huxiaoheigame@gmail.com
* Created on 2021/09/19
* Copyright (c) 2021 虎小黑
****************************************************************/

#include "coroutine.h"

#include <atomic>

#include "config.h"
#include "log.h"
#include "macro.h"

namespace tigerkin {

static std::atomic<uint64_t> s_co_id{0};
static std::atomic<uint64_t> s_co_cnt{0};

static thread_local Coroutine *t_co = nullptr;
static thread_local Coroutine::ptr t_threadCo = nullptr;

static ConfigVar<uint32_t>::ptr g_co_stack_size = Config::Lookup<uint32_t>("co.stackSize", 1024 * 1024, "CoStackSize");

class MallocStackAllocator {
   public:
    static void *Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void *vp, size_t size) {
        return free(vp);
    }
};

typedef MallocStackAllocator StackAllocator;

Coroutine::Coroutine() {
    m_state = State::EXECING;
    SetThis(this);
    if (getcontext(&m_ctx)) {
        TIGERKIN_ASSERT2(false, "GET CONTEXT ERROR");
    }
    ++s_co_cnt;
}

Coroutine::Coroutine(std::function<void()> cb, size_t stackSize)
    : m_id(++s_co_id), m_cb(cb) {
    if (!t_threadCo) {
        Coroutine::ptr mainCo(new Coroutine);
        t_threadCo = mainCo;
    }
    ++s_co_cnt;
    m_stackSize = stackSize ? stackSize : g_co_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stackSize);
    if (getcontext(&m_ctx)) {
        TIGERKIN_ASSERT2(false, "GET CONTEXT ERROR");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stackSize;
    makecontext(&m_ctx, &Coroutine::MainFunc, 0);
}

Coroutine::~Coroutine() {
    --s_co_cnt;
    if (m_stack) {
        TIGERKIN_ASSERT(m_state == State::INIT ||
                        m_state == State::TERMINAL ||
                        m_state == State::EXCEPT);
        StackAllocator::Dealloc(m_stack, m_stackSize);
    } else {
        TIGERKIN_ASSERT(!m_cb);
        TIGERKIN_ASSERT(m_state == State::EXECING);
        Coroutine *co = t_co;
        if (co == this) {
            SetThis(nullptr);
        }
    }
    // TIGERKIN_LOG_DEBUG(TIGERKIN_LOG_NAME(SYSTEM)) << "DESTROY COROUTIN: " << m_id;
}

void Coroutine::reset(std::function<void()> cb) {
    TIGERKIN_ASSERT(m_stack);
    TIGERKIN_ASSERT(m_state == State::INIT ||
                    m_state == State::TERMINAL ||
                    m_state == State::EXCEPT);
    m_cb = cb;
    if (getcontext(&m_ctx)) {
        TIGERKIN_ASSERT2(false, "GET CONTEXT ERROR");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stackSize;
    makecontext(&m_ctx, &Coroutine::MainFunc, 0);
    m_state = State::INIT;
}

void Coroutine::resume() {
    SetThis(this);
    TIGERKIN_ASSERT(m_state != State::EXECING);
    m_state = State::EXECING;
    if (swapcontext(&t_threadCo->m_ctx, &m_ctx)) {
        TIGERKIN_ASSERT2(false, "SWAP CONTEXT ERROR");
    }
}

void Coroutine::yield() {
    SetThis(t_threadCo.get());
    m_state = m_state == State::EXECING ? State::YIELD : m_state;
    if (swapcontext(&m_ctx, &t_threadCo->m_ctx)) {
        TIGERKIN_ASSERT2(false, "SWAP CONTEXT ERROR");
    }
}

void Coroutine::SetThis(Coroutine *co) {
    t_co = co;
}

Coroutine::ptr Coroutine::GetThis() {
    if (t_co) {
        return t_co->shared_from_this();
    }
    if (!t_threadCo) {
        Coroutine::ptr mainCo(new Coroutine);
        t_threadCo = mainCo;
    }
    return t_co->shared_from_this();
}

uint64_t Coroutine::GetCoId() {
    if (t_co) {
        return t_co->getId();
    }
    return 0;
}

void Coroutine::Yield() {
    Coroutine::ptr curCo = GetThis();
    TIGERKIN_ASSERT2(curCo != t_threadCo, "YIELD THE MAIN COROUTINE FROM CURRENT THREAD IS ERROR");
    curCo->yield();
}

uint64_t Coroutine::TotalCo() {
    return s_co_cnt;
}

void Coroutine::MainFunc() {
    Coroutine::ptr curCo = GetThis();
    try {
        TIGERKIN_ASSERT(curCo);
        curCo->m_cb();
        curCo->m_cb = nullptr;
        curCo->m_state = State::TERMINAL;
    } catch (std::exception &ex) {
        curCo->m_state = State::EXCEPT;
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "COROUTINE EXCEPT: " << ex.what();
    } catch (...) {
        curCo->m_state = State::EXCEPT;
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "COROUTINE EXCEPT";
    }
    Coroutine *rawPtr = curCo.get();
    curCo.reset();
    rawPtr->yield();
}

}  // namespace tigerkin
