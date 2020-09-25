#pragma once
#include <AUI/Thread/IEventLoop.h>

class AWindow;

class API_AUI_VIEWS AWindowManager: public IEventLoop {
friend class AWindow;
private:
    IEventLoop::Handle mHandle;
    ADeque<_<AWindow>> mWindows;
    bool mLoopRunning = false;
#ifdef __linux
    AMutex mXNotifyLock;
    AConditionVariable mXNotifyCV;
#endif

public:
    AWindowManager();
    ~AWindowManager() override;

    void notifyProcessMessages() override;
    void loop() override;
};

