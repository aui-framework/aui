#pragma once


#include "IEventLoop.h"
#include "AMutex.h"
#include "AConditionVariable.h"

/**
 * @brief Default event loop implementation.
 * @ingroup core
 */
class API_AUI_CORE AEventLoop: public IEventLoop {
public:
    ~AEventLoop() override;

    void notifyProcessMessages() override;

    /**
     * @brief Default loop.
     * @details
     * Until mRunning = true, calls iteraton() infinitely. The loop can be broken via stop() method.
     */
    void loop() override;

    void stop() {
        mRunning = false;
        notifyProcessMessages();
    }

    /*
     * @brief One loop iteration.
     * @details
     * Useful to define custom loops.
     *
     * If no tasks for current thread available, blocks the thread execution.
     */
    void iteration();

private:
    AMutex mMutex;
    AConditionVariable mCV;
    bool mNotified = false;
    bool mRunning = false;
};
