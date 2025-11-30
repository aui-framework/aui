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
#include <exception>

#include "AString.h"
#include "AUI/Reflect/AReflect.h"
#include <AUI/Common/AVector.h>
#include <AUI/Platform/AStacktrace.h>
#include <AUI/Traits/strings.h>

/**
 * @brief Abstract AUI exception.
 * @ingroup core
 * @details
 * AException is the base exception type in AUI. It extends `std::exception` with automatic
 * [stack trace capture](astacktrace.md), lazy message formatting, and exception nesting (causal chains).
 *
 * At the point of throwing, [stack trace](astacktrace.md) does not resolve symbol names and source lines. These are
 * resolved when exception is printed.
 *
 * ```cpp title="main.cpp"
 * static constexpr auto LOG_TAG = "MyApp";
 *
 * int main() {
 *   try {
 *      throw AException("test");
 *   } catch (const AException& e) {
 *      ALogger::info(LOG_TAG) << "Operation failed: " << e;
 *   }
 * }
 * ```
 *
 * ``` title="Possible output"
 * Operation failed: test
 * - 0x00000000700000f0 main at main.cpp:5
 * ```
 *
 * Exceptions are chained automatically with `std::current_exception()`. This means you can `throw` another exception in
 * `catch` block to populate the error message since callers tend to have wider context. If `AException` has a nested
 * exception, it will be printed as well.
 *
 * ```cpp title="main.cpp"
 * static constexpr auto LOG_TAG = "MyApp";
 *
 * int safeDiv(int a, int b) {
 *    if (b == 0) {
 *        throw AException("divider is zero");
 *    }
 *    return a / b;
 * }
 *
 * int readData(AStringView path) {
 *    try {
 *        AFileInputStream fis(path);
 *        int a, b;
 *        fis >> a >> b;
 *        return safeDiv(a, b);
 *    } catch (const AException&) {
 *        // add file path information. the original exception will
 *        // be carried by the new one.
 *        throw AException("can't read file \"{}\""_format(path));
 *    }
 * }
 *
 * int main() {
 *    try {
 *       auto data = readData("data.bin");
 *       // ...
 *    } catch (const AException& e) {
 *       ALogger::err(LOG_TAG) << "Operation failed: " << e;
 *    }
 * }
 * ```
 *
 * ``` title="Possible output"
 * Operation failed: can't read file "data.bin"
 * - 0x0000000070000f00 readData at main.cpp:19
 * - 0x00000000700000f0 main at main.cpp:25
 * Caused by: divider is zero
 * - 0x0000000070001000 safeDiv at main.cpp:5
 * - 0x0000000070000f00 readData at main.cpp:15
 * - 0x00000000700000f0 main at main.cpp:25
 * ```
 *
 */
class API_AUI_CORE AException : public std::exception {
    mutable AOptional<std::string> mMessage;

public:
    AException() : mStacktrace(AStacktrace::capture(2)) {}
    AException(AStacktrace stacktrace) : mStacktrace(std::move(stacktrace)) {}

    AException(const AString& message) : AException() { mMessage = message.toStdString(); }

    AException(const AException& exception) = default;
    AException(AException&& exception) noexcept = default;

    AException(const AString& message, std::exception_ptr causedBy = std::current_exception(), AStacktrace stacktrace = AStacktrace::capture(2))
      : mStacktrace(std::move(stacktrace)), mCausedBy(std::move(causedBy)) { // NOLINT(*-throw-keyword-missing)
        mMessage = message.toStdString();
    }

    virtual AString getMessage() const noexcept { return mMessage ? mMessage->c_str() : "<no message>"; }

    ~AException() noexcept override = default;

    const char* what() const noexcept override {
        if (!mMessage)
            mMessage = getMessage().toStdString();
        return mMessage->c_str();
    }

    const AStacktrace& stacktrace() const noexcept { return mStacktrace; }

    [[nodiscard]]
    const std::exception_ptr& causedBy() const noexcept {
        return mCausedBy;
    }

private:
    AStacktrace mStacktrace;
    std::exception_ptr mCausedBy = std::current_exception();
};

inline std::ostream& operator<<(std::ostream& o, const AException& e) noexcept {
    o << "(" << AReflect::name(&e) << ") " << e.getMessage() << std::endl << e.stacktrace();
    if (e.causedBy()) {
        o << "Caused by: ";
        try {
            std::rethrow_exception(e.causedBy());
        } catch (const AException& e) {
            o << e;
        } catch (const std::exception& e) {
            o << e.what();
        } catch (...) {
            o << "unknown exception";
        }
    }
    return o;
}