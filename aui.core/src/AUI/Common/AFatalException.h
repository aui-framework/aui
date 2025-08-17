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

//
// Created by alex2772 on 1/14/22.
//

#pragma once

#include "AException.h"

/**
 * @brief An exception that thrown when non-c++ unhandled error occurs (i.e. access violation).
 * @ingroup core
 * @details
 * !!! note
 *
 *     AFatalException::setGlobalHandler is supported on all platforms, but translation to AFatalError is not as it
 *     uses compiler-specific flags and hacks. See [AFatalException_Platform_support] for further info.
 *
 * Translation to throwing AFatalException recovers the application from crashed state, providing you ability to handle
 * such errors and continue normal application execution. However, still recommended to handle such errors with
 * setGlobalHandler() to at least save user data.
 *
 * # Platform support { #AFatalException_Platform_support }
 * | Platform | Translation to throwing AFatalException | setGlobalHandler() |
 * |----------|-----------------------------------------|--------------------|
 * | Windows  | MSVC                                    | +                  |
 * | Linux    | GCC                                     | +                  |
 * | macOS    | -                                       | +                  |
 * | Android  | GCC                                     | +                  |
 * | iOS      | -                                       | +                  |
 */
class API_AUI_CORE AFatalException: public AException {
public:
    using Handler = std::function<void(AFatalException*)>;

    explicit AFatalException(std::string_view signalName, int nativeSignalId);

    AString getMessage() const noexcept override;

    /**
     * @return Address where does the fatal exception occurred.
     */
    void* address() const {
        return mAddress;
    }

    /**
     * @return Native signal id.
     */
    int nativeSignalId() const {
        return mNativeSignalId;
    }

    /**
     * @return Signal name.
     */
    std::string_view signalName() const {
        return mSignalName;
    }

    /**
     * @brief Sets handler for fatal exceptions.
     * @param globalHandler handler
     * @details
     * The handler is called inside OS's signal callback, thus, it's not recommended to use the following operations
     * during callback execution:
     * <ul>
     *   <li>I/O (printf, logging, fopen, fread, fwrite, etc...)</li>
     *   <li>Heap routines (malloc, free, new, delete), including heap-based containers (std::vector, std::list,
     *   std::queue, etc...)</li>
     *   <li>System calls (time, getcwd, etc...)</li>
     * </ul>
     * Basically, you may want to define global AOptional&lt;AFatalException&gt; and store the copy of exception in
     * order to process it outside of the callback.
     *
     * The callback may be used for handling and sending telemetry data (however, be aware of using unrecommended
     * operations).
     */
    static void setGlobalHandler(Handler globalHandler) {
        handler() = std::move(globalHandler);
    }

private:
    void* mAddress;
    std::string_view mSignalName;
    int mNativeSignalId;

    static Handler& handler();
};