// AUI Framework - Declarative UI toolkit for modern C++17
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

#pragma once

#include <AUI/Traits/values.h>

#if AUI_PLATFORM_WIN
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>
#include <spawn.h>
#endif


class Pipe: public aui::noncopyable {
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


    [[nodiscard]]
    pipe_t in() const noexcept {
        return mIn;
    }

    [[nodiscard]]
    pipe_t out() const noexcept {
        return mOut;
    }

    void closeIn() noexcept;
    void closeOut() noexcept;

    [[nodiscard]]
    pipe_t stealIn() noexcept {
        auto copy = mIn;
        mIn = 0;
        return copy;
    }

    [[nodiscard]]
    pipe_t stealOut() noexcept {
        auto copy = mOut;
        mOut = 0;
        return copy;
    }

private:
    pipe_t mOut;
    pipe_t mIn;
};