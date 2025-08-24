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

#include <AUI/Traits/values.h>
#include <AUI/IO/IInputStream.h>
#include <AUI/IO/IOutputStream.h>

#if AUI_PLATFORM_WIN
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>
#include <spawn.h>
#endif


/**
 * @brief Native pipe RAII wrapper.
 * @details
 * @specificto{windows}
 * On Windows, implemented with CreateNamedPipe.
 *
 * On *nix, implemented with pipe(2).
 */
class API_AUI_CORE Pipe final: public aui::noncopyable, public IInputStream, public IOutputStream {
public:
#if AUI_PLATFORM_WIN
    using pipe_t = HANDLE;
#else
    using pipe_t = int;
#endif

    Pipe();
    Pipe(Pipe&& rhs) noexcept {
        operator=(std::move(rhs));
    }
    ~Pipe();

    Pipe& operator=(Pipe&& rhs) noexcept {
        mIn = rhs.mIn;
        mOut = rhs.mOut;
        rhs.mIn = rhs.mOut = static_cast<pipe_t>(0);
        return *this;
    }


    /**
     * @brief Out pipe. Also known as `pipe[0]`.
     */
    [[nodiscard]]
    pipe_t out() const noexcept {
        return mOut;
    }

    /**
     * @brief In pipe. Also known as `pipe[1]`.
     */
    [[nodiscard]]
    pipe_t in() const noexcept {
        return mIn;
    }

    /**
     * @brief Close out. Also known as `close(pipe[0])`
     */
    void closeOut() noexcept;

    /**
     * @brief Close in. Also known as `close(pipe[1])`
     */
    void closeIn() noexcept;

    /**
     * @brief Steals ownership of the out pipe outside of the Pipe class.
     * @details
     * Resets the pipe value to zero. Caller is responsible for closing the pipe.
     */
    [[nodiscard]]
    pipe_t stealOut() noexcept {
        auto copy = mOut;
        mOut = 0;
        return copy;
    }

    /**
     * @brief Steals ownership of the in pipe outside of the Pipe class.
     * @details
     * Resets the pipe value to zero. Caller is responsible for closing the pipe.
     */
    [[nodiscard]]
    pipe_t stealIn() noexcept {
        auto copy = mIn;
        mIn = 0;
        return copy;
    }
    size_t read(char* dst, size_t size) override;
    void write(const char* src, size_t size) override;

private:
    /**
     * @brief Out pipe. Also known as `pipe[0]`.
     */
    pipe_t mOut;

    /**
     * @brief In pipe. Also known as `pipe[1]`.
     */
    pipe_t mIn;
};