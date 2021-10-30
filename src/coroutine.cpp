/*****************************************************************
* Description coroutine
* Email huxiaoheigame@gmail.com
* Created on 2021/09/19
* Copyright (c) 2021 虎小黑
****************************************************************/

#include "coroutine.h"

#include <atomic>
#include <map>
#include <stack>

#include "config.h"
#include "macro.h"
#include "util.h"

namespace tigerkin {

static std::atomic<uint64_t> s_co_id{0};
static std::atomic<uint64_t> s_co_cnt{0};
static std::atomic<uint64_t> s_stack_id{0};

static thread_local Coroutine *t_cur_co = nullptr;
static thread_local Coroutine::ptr t_main_co = nullptr;
static thread_local std::map<uint64_t, std::stack<Coroutine *> *> t_map_co_stack;

static ConfigVar<uint32_t>::ptr g_co_stack_size = Config::Lookup<uint32_t>("tigerkin.coroutine.stackSize", 1024 * 1024, "StackSize");

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
    if (!t_main_co) {
        Coroutine::ptr mainCo(new Coroutine);
        t_main_co = mainCo;
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
        if (t_main_co && t_cur_co == this) {
            SetThis(nullptr);
        }
    }
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
    if (m_stackId > 0 || (t_cur_co->m_state == State::EXECING && t_cur_co != t_main_co.get())) {
        m_stackId = m_stackId > 0 ? m_stackId : t_cur_co->m_stackId;
        if (t_map_co_stack.find(m_stackId) == t_map_co_stack.end()) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "Can not find coroutine stack by m_stackId \n"
                                                          << BacktraceToString(10);
            return;
        }
        std::stack<Coroutine *> *coStack = t_map_co_stack.at(m_stackId);
        if (coStack->top() != this) {
            coStack->top()->m_state = State::YIELD;
            m_ctx.uc_link = &coStack->top()->m_ctx;
            coStack->push(this);
        }
        if (coStack->top()->m_state == State::EXECING) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "The coroutine is already resumed \n"
                                                          << BacktraceToString(10);
            return;
        }
        if (coStack->top()->m_state == State::TERMINAL || coStack->top()->m_state == State::EXCEPT) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "The coroutine is already exited \n"
                                                          << BacktraceToString(10);
            return;
        }
        m_state = State::EXECING;
        SetThis(this);
        int swapFail = 0;
        if (t_main_co->m_state == State::EXECING) {
            t_main_co->m_state = State::YIELD;
            swapFail = swapcontext(&t_main_co->m_ctx, &m_ctx);
        } else {
            swapFail = swapcontext(m_ctx.uc_link, &m_ctx);
        }
        if (swapFail) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "SWAP CONTEXT ERROR \n"
                                                          << BacktraceToString(10);
            m_state = State::EXCEPT;
            t_map_co_stack.at(m_stackId)->pop();
            if (t_map_co_stack.at(m_stackId)->empty()) {
                delete t_map_co_stack.at(m_stackId);
                t_map_co_stack.erase(m_stackId);
                SetThis(t_main_co.get());
                t_main_co->m_state = State::EXECING;
                setcontext(&t_main_co->m_ctx);
            } else {
                t_map_co_stack.at(m_stackId)->top()->m_state = State::EXECING;
                SetThis(t_map_co_stack.at(m_stackId)->top());
                setcontext(&t_map_co_stack.at(m_stackId)->top()->m_ctx);
            }
        }
    } else {
        m_ctx.uc_link = &t_main_co->m_ctx;
        std::stack<Coroutine *> *coStack = new std::stack<Coroutine *>;
        coStack->push(this);
        m_stackId = ++s_stack_id;
        t_map_co_stack.insert({m_stackId, coStack});
        SetThis(this);
        t_main_co->m_state = State::YIELD;
        m_state = State::EXECING;
        if (swapcontext(&t_main_co->m_ctx, &m_ctx)) {
            TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "SWAP CONTEXT ERROR \n"
                                                          << BacktraceToString(10);
            t_map_co_stack.erase(m_stackId);
            delete coStack;
            SetThis(t_main_co.get());
            t_main_co->m_state = State::EXECING;
            setcontext(&t_main_co->m_ctx);
        }
    }
}

void Coroutine::yield() {
    if (t_map_co_stack.find(m_stackId) == t_map_co_stack.end()) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "Can not find coroutine stack by m_stackId \n"
                                                      << BacktraceToString(10);
        return;
    }
    if (this != t_map_co_stack.at(m_stackId)->top()) {
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "Only the coroutine from stack top can yield \n"
                                                      << BacktraceToString(10);
        return;
    }
    SetThis(t_main_co.get());
    m_state = m_state == State::EXECING ? State::YIELD : m_state;
    t_main_co->m_state = State::EXECING;
    if (swapcontext(&m_ctx, &t_main_co->m_ctx)) {
        TIGERKIN_ASSERT2(false, "SWAP CONTEXT ERROR");
    }
}

void Coroutine::SetThis(Coroutine *co) {
    t_cur_co = co;
}

Coroutine::ptr Coroutine::GetThis() {
    if (t_cur_co) {
        return t_cur_co->shared_from_this();
    }
    if (!t_main_co) {
        Coroutine::ptr mainCo(new Coroutine);
        t_main_co = mainCo;
    }
    return t_cur_co->shared_from_this();
}

uint64_t Coroutine::GetCoId() {
    if (t_cur_co) {
        return t_cur_co->getId();
    }
    return 0;
}

void Coroutine::Resume(uint64_t stackId) {
    if (t_map_co_stack.find(stackId) == t_map_co_stack.end()) {
        if (t_main_co->m_state == State::EXECING) {
            return;
        }
        t_main_co->m_state = State::EXECING;
        setcontext(&t_main_co->m_ctx);
        return;
    }
    t_map_co_stack.at(stackId)->top()->resume();
}

void Coroutine::Yield() {
    Coroutine::ptr curCo = GetThis();
    TIGERKIN_ASSERT2(curCo != t_main_co, "YOU CAN NOT YIELD THE MAIN COROUTINE");
    curCo->yield();
}

size_t Coroutine::CoCnt() {
    return s_co_cnt;
}

size_t Coroutine::CoStackCnt() {
    return t_map_co_stack.size();
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
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "COROUTINE EXCEPT: \n"
                                                      << ex.what() << "\n"
                                                      << BacktraceToString(10);
    } catch (...) {
        curCo->m_state = State::EXCEPT;
        TIGERKIN_LOG_ERROR(TIGERKIN_LOG_NAME(SYSTEM)) << "COROUTINE EXCEPT\n"
                                                      << BacktraceToString(10);
    }
    t_map_co_stack.at(curCo->m_stackId)->pop();
    if (t_map_co_stack.at(curCo->m_stackId)->empty()) {
        delete t_map_co_stack.at(curCo->m_stackId);
        t_map_co_stack.erase(curCo->m_stackId);
        SetThis(t_main_co.get());
    } else {
        SetThis(t_map_co_stack.at(curCo->m_stackId)->top());
    }
    curCo.reset();
    t_cur_co->m_state = State::EXECING;
    if (setcontext(&t_cur_co->m_ctx)) {
        std::cout << "error" << std::endl;
    }
    std::cout << "error" << std::endl;
}

}  // namespace tigerkin
