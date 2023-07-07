// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

//
// Created by alex2772 on 1/14/22.
//

#pragma once

#include "AException.h"

/**
 * @brief An exception that thrown when non-c++ unhandler error occurs (i.e. access violation).
 * @ingroup core
 * @details
 * Your application may handle AFatalException and continue normal execution.
 *
 * @note AFatalException::setGlobalHandler is supported on all platforms, but translation to AFatalError is not. See
 * @ref "Platform support" for further info.
 *
 * # Platform support
 * | Platform | Translation to throwing AFatalException | setGlobalHandler |
 * |----------|-----------------------------------------|------------------|
 * | Windows  | MSVC                                    | +                |
 * | Linux    | GCC                                     | +                |
 * | macOS    | -                                       | +                |
 * | Android  | GCC                                     | +                |
 * | iOS      | -                                       | +                |
 */
class API_AUI_CORE AFatalException: public AException {
public:
    using Handler = std::function<void(AFatalException*)>;

    explicit AFatalException(void* address, std::string_view signalName):
        mAddress(address), mSignalName(signalName) // avoiding unrecommended operations as much as possible
        {
        if (handler())
            handler()(this);
    }

    AString getMessage() const noexcept override;

    void* getAddress() const {
        return mAddress;
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
     */
    static void setGlobalHandler(Handler globalHandler) {
        handler() = std::move(globalHandler);
    }

private:
    void* mAddress;
    std::string_view mSignalName;

    static Handler& handler();
};