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

#include "AThread.h"

#include <cassert>
#include <AUI/Common/AException.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Logging/ALogger.h>
#include "AUI/Platform/AStacktrace.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/Thread/AMutexWrapper.h"
#include "IEventLoop.h"
#include <AUI/Thread/AConditionVariable.h>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

#if AUI_PLATFORM_WIN
#include <Windows.h>
#include <processthreadsapi.h>
#include <AUI/Platform/ErrorToException.h>
#include <AUI/Platform/win32/WinHandle.h>

void setThreadNameImpl(HANDLE handle, const AString& name) {
    static struct SetThreadDescription {
    private:
        using T = HRESULT(WINAPI*)(HANDLE, PCWSTR);
        T mPtr = nullptr;

    public:
        SetThreadDescription() {
            auto hKernel32 = GetModuleHandleA("kernel32.dll");
            if (hKernel32) {
                mPtr = reinterpret_cast<T>(GetProcAddress(hKernel32, "SetThreadDescription"));
            }
        }

        operator bool() const { return mPtr != nullptr; }

        HRESULT operator()(HANDLE thread, PCWSTR name) { return mPtr(thread, name); }
    } s;
    if (s) {
        s(handle, aui::win32::toWchar(name));
    }
}
#else
#include <signal.h>
#if !AUI_PLATFORM_ANDROID && !AUI_PLATFORM_EMSCRIPTEN
#include <execinfo.h>
#endif
#include <pthread.h>
#endif
namespace aui::impl::AThread {
static AMutexWrapper<AMap<std::thread::id, std::function<void()>>>& payloads() {
    static AMutexWrapper<AMap<std::thread::id, std::function<void()>>> d;
    return d;
}
void executeForcedPayload() {
    std::unique_lock lock(payloads());
    if (auto it = payloads()->contains(std::this_thread::get_id())) {
        if (it->second) {
            it->second();
            it->second = {};
        }
    }
}
}   // namespace aui::impl::AThread
class CurrentThread : public AAbstractThread {
public:
    using AAbstractThread::AAbstractThread;
};

AAbstractThread::AAbstractThread(const id& id) noexcept : mId(id) {}

AStacktrace AAbstractThread::threadStacktrace() const {
    if (std::this_thread::get_id() == mId) {
        return AStacktrace::capture(1);
    }
    auto& payloads = aui::impl::AThread::payloads();
    std::unique_lock lock(payloads);
    AFuture<AStacktrace> future;
    payloads.value()[mId] = [future] {
#if AUI_PLATFORM_WIN
        future.supplyValue(AStacktrace::capture(1));
#else
        future.supplyValue(AStacktrace::capture(6));
#endif
    };
    lock.unlock();
#if AUI_PLATFORM_WIN
    aui::win32::Handle h = OpenThread(
        THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, false, reinterpret_cast<const DWORD&>(mId));
    if (!h) {
        aui::impl::lastErrorToException("OpenThread returned null handle");
    }
    if (SuspendThread(h) == -1) {
        aui::impl::lastErrorToException("SuspendThread failed");
    }
    CONTEXT context;
    aui::zero(context);
    context.ContextFlags = CONTEXT_FULL;
    if (GetThreadContext(h, &context) == 0) {
        aui::impl::lastErrorToException("GetThreadContext failed");
    }
    ARaiiHelper contextReturner = [&] { SetThreadContext(h, &context); };
#if AUI_ARCH_ARM_64
#define REG_SP Sp
#define REG_IP Pc
#elif AUI_ARCH_X86_64
#define REG_SP Rsp
#define REG_IP Rip
#else
#define REG_SP Esp
#define REG_IP Eip
#endif
    {
        auto contextCopy = context;
        auto& stackPointer = (reinterpret_cast<std::uintptr_t*&>(contextCopy.REG_SP));
        *(--stackPointer) = contextCopy.REG_IP;
        contextCopy.REG_IP =
            reinterpret_cast<const decltype(contextCopy.REG_IP)>(&aui::impl::AThread::executeForcedPayload);
        SetThreadContext(h, &contextCopy);
        if (ResumeThread(h) == -1) {
            aui::impl::lastErrorToException("ResumeThread failed");
        }
    }
//		SuspendThread()
#else
    if (pthread_kill(reinterpret_cast<const pthread_t&>(mId), SIGUSR1) != 0) {
        throw AException("unable to acquire other thread stacktrace: pthread_kill failed");
    }
#endif
    return *future;
}

_<AAbstractThread>& AAbstractThread::threadStorage() {
    thread_local _<AAbstractThread> thread;
    return thread;
}

AAbstractThread::~AAbstractThread() = default;

bool AAbstractThread::isInterrupted() {
    // abstract thread could not be interrupted.
    return false;
}

void AAbstractThread::interrupt() {}

void AAbstractThread::resetInterruptFlag() {}

void AThread::start() {
    assert(mThread == nullptr);
    auto t = aui::ptr::shared_from_this(this);
    updateThreadName();
    mThread = new std::thread([&, t]() {
        threadStorage() = t;
        auto f = std::move(mFunctor);
        mFunctor = nullptr;
        mId = std::this_thread::get_id();
        try {
            f();
        } catch (const AException& e) {
            ALogger::err("uncaught exception: " + e.getMessage());
        } catch (Interrupted) {
        }
    });
}

void AThread::interrupt() {
    mInterrupted = true;
    std::unique_lock lock(mCurrentCV.mutex);
    AUI_NULLSAFE(mCurrentCV.cv)->notify_all();
}

void AThread::sleep(std::chrono::milliseconds duration) {
    AConditionVariable cv;
    AMutex mutex;
    std::unique_lock lock(mutex);
    // the condition variable helps with thread interruption.
    cv.wait_for(lock, duration);
}

AAbstractThread::id AAbstractThread::getId() const { return mId; }

const _<AAbstractThread>& AThread::current() {
    auto& t = threadStorage();
    if (t == nullptr)   // abstract thread
    {
        t = aui::ptr::manage_shared(new CurrentThread(std::this_thread::get_id()));
    }

    return t;
}

void AThread::interruptionPoint() {
    if (current()->isInterrupted()) {
        current()->resetInterruptFlag();
        throw Interrupted();
    }
}

bool AThread::isInterrupted() { return mInterrupted; }

void AThread::resetInterruptFlag() { mInterrupted = false; }

void AThread::join() {
    if (mThread->get_id() == std::this_thread::get_id()) {
        throw AException("AThread::join to the self thread");
    }
    if (!mThread->joinable()) {
        return;
    }
    mThread->join();
}

void AAbstractThread::enqueue(AMessageQueue<>::Message f) {
    mMessageQueue.enqueue(std::move(f));
    if (mCurrentEventLoop) {
        std::unique_lock lock(mEventLoopLock);
        if (mCurrentEventLoop) {
            mCurrentEventLoop->notifyProcessMessages();
        }
    }
}

void AAbstractThread::processMessagesImpl() {
    AUI_ASSERTX(
        mId == std::this_thread::get_id(), "AAbstractThread::processMessages() should not be called from other thread");
    mMessageQueue.processMessages();
}

AThread::~AThread() {
    if (!mThread) {
        return;
    }
    AThread::interrupt();
    if (mThread->get_id() == std::this_thread::get_id()) {
        mThread->detach();
        delete mThread;
    } else {
        if (mThread->joinable())
            mThread->join();
        delete mThread;
    }
    mThread = nullptr;
}

void AThread::detach() {
    mThread->detach();
    delete mThread;
    mThread = nullptr;
}

void AAbstractThread::updateThreadName() noexcept {
    if (!mThreadName.empty()) {
#if AUI_PLATFORM_WIN
        setThreadNameImpl((HANDLE) GetCurrentThread(), mThreadName);
#elif AUI_PLATFORM_APPLE
        auto name = mThreadName.toStdString();
        AUI_ASSERTX(name.size() < 16, "on unix thread name restricted to 15 chars length");
        pthread_setname_np(name.c_str());
#elif AUI_PLATFORM_ANDROID || AUI_PLATFORM_LINUX
        auto name = mThreadName.toStdString();
        AUI_ASSERTX(name.size() < 16, "on unix thread name restricted to 15 chars length");
        pthread_setname_np(pthread_self(), name.c_str());
#endif
    }
}

AThread::AThread(std::function<void()> functor) : mFunctor(std::move(functor)) {}

bool AAbstractThread::messageQueueEmpty() noexcept { return mMessageQueue.messages().empty(); }

const _<AAbstractThread>& AThread::main() noexcept {
    static auto main = current(); // initialized by AUI_ENTRY.
    return main;
}
