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

#include <AUI/Thread/AMutex.h>
#include <AUI/Common/ADeque.h>

/**
 * @brief Universal thread-safe message (callback) queue implementation.
 * @ingroup core
 * @details
 * Thread-safe message queue. The implementation supports enqueueing new messages inside the callbacks.
 */
template<typename Mutex = AMutex, typename... Args>
class AMessageQueue {
public:
    using Message = std::function<void(Args...)>;

    /**
     * @brief Add message to the queue to process in processMessages().
     */
    void enqueue(Message message) {
		std::unique_lock lock(mSync);
		mMessages << std::move(message);
    }

    /**
     * @brief Process messages submitted by enqueue method.
     */
    void processMessages(Args... args) {
        // cheap lookahead that does not require mutex lock.
        if (mMessages.empty()) {
            return;
        }
        std::unique_lock lock(mSync);
        while (!mMessages.empty()) {
            {
                auto queue = std::move(mMessages);
                lock.unlock();
                for (auto &message: queue) {
                    message(args...);
                }
            } // destroy queue before mutex lock

            // cheap lookahead that does not require mutex lock.
            if (mMessages.empty()) {
                return;
            }

            lock.lock();
        }
    }

    [[nodiscard]]
    Mutex& sync() noexcept {
        return mSync;
    }

    [[nodiscard]]
    ADeque<Message>& messages() noexcept {
        return mMessages;
    }

    [[nodiscard]]
    const Mutex& sync() const noexcept {
        return mSync;
    }

    [[nodiscard]]
    const ADeque<Message>& messages() const noexcept {
        return mMessages;
    }

private:
    ADeque<Message> mMessages;

    /**
     * @brief Message queue mutex.
     */
    Mutex mSync;
};
