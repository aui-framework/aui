// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
