#pragma once


#include "IEventLoop.h"
#include "AMutex.h"
#include "AConditionVariable.h"

/**
 * @brief Default event loop implementation.
 * @ingroup core
 */
class AEventLoop: public IEventLoop {
public:
    ~AEventLoop() override;

    void notifyProcessMessages() override;

    void loop() override;

private:
    AMutex mMutex;
    AConditionVariable mCV;
    bool mLoopFlag = false;
};
