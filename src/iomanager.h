/*****************************************************************
* Description IO管理器(基于epoll)
* Email huxiaoheigame@gmail.com
* Created on 2021/10/13
* Copyright (c) 2021 虎小黑
****************************************************************/

#ifndef __TIGERKIN_IOMANAGER_H__
#define __TIGERKIN_IOMANAGER_H__

#include <string>

#include "scheduler.h"
#include "timer.h"

namespace tigerkin {

class IOManager : public Scheduler, public TimerManager {
   public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef ReadWriteLock RWMutex;
    typedef int FdId;

    enum Event {
        NONE = 0x0,
        READ = 0x001,
        WRITE = 0x004
    };

    enum IOManagerCode {
        ADD_EVENT_ERR = 0,
        ADD_EVENT_SUC = -1,
    };

   private:
    struct FdContext {
        typedef MutexLock Mutex;
        struct EventContext {
            Scheduler *scheduler = nullptr;
            uint64_t threadId = 0;
            Coroutine::ptr co;
            std::function<void()> cb; 
        };
        EventContext & getEventContext(const Event event);
        void resetContext(EventContext &ctx);
        void triggerEvent(const Event event);

        EventContext read;
        EventContext write;
        FdId fd = 0;
        Event events = Event::NONE;
        Mutex mutex;
    };

   public:
    IOManager(size_t threadCnt = 1, bool useCaller = true, const std::string &name = "");
    ~IOManager();
    IOManagerCode addEvent(FdId fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(FdId fd, Event event);
    bool cancelEvent(FdId fd, Event event);
    bool cancelAllEvent(FdId fd);

    static IOManager *GetThis();

   protected:
    bool stopping() override;
    bool canEarlyClosure() override;
    void tickle() override;
    void idle() override;
    void onTimerRefresh() override;

    void fdContextsResize(size_t size);

   private:
    int m_epfd = 0;
    int m_tickleFds[2];
    std::atomic<size_t> m_pendingEventCount = {0};
    RWMutex m_mutex;
    std::vector<FdContext *> m_fdContexts;
};

}  // namespace tigerkin

#endif