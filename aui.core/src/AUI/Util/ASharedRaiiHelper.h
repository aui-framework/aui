/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#include <functional>
#include <utility>
#include <AUI/Common/AObject.h>

/**
 * @brief Shared completion marker for multithreaded callback-based future handling.
 * @ingroup core
 *
 * @details
 * ASharedRaiiHelper is intended to track the completion of multiple futures, exploiting the destruction
 * of AFuture::onSuccess or AFuture::onError callbacks when they called. This approach allows to easily integrate
 * ASharedRaiiHelper to the existing code.
 *
 * Note that the callback will be called even if onSuccess is not.
 *
 * ```cpp
 * auto marker = ASharedRaiiHelper::make([] {
 *     // will be called when all futures called onSuccess or destroyed
 * });
 * auto future = AUI_THREADPOOL {
 *     // hard work
 * }
 * //                      VVVVVV ASharedRaiiHelper should be captured!
 * future.onSuccess([this, marker]() {
 *     // handle the hard work
 * });
 *
 * // keep the future alive in AUI_THREADPOOL holder
 * mAsyncHolder << future;
 *
 * ```
 */
class ASharedRaiiHelper {
public:
    static _<ASharedRaiiHelper> make(std::function<void()> callback) {
        return aui::ptr::manage_shared(new ASharedRaiiHelper(std::move(callback)));
    }

    ~ASharedRaiiHelper() {
        callback();
    }

private:
    std::function<void()> callback;

    explicit ASharedRaiiHelper(std::function<void()> callback) : callback(std::move(callback)) {}
};


