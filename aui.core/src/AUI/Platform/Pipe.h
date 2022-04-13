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
    Pipe(Pipe&& rhs) noexcept: mIn(rhs.mIn), mOut(rhs.mOut) {
        rhs.mIn = rhs.mOut = static_cast<pipe_t>(0);
    }
    ~Pipe();

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

private:
    pipe_t mOut;
    pipe_t mIn;
};