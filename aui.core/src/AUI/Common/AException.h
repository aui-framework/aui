/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
 * Unlike std::exception, AException is capable to capture stack traces and efficiently output them to std::ostream.
 * Also exception nesting is possible (via causedBy()).
 */
class API_AUI_CORE AException : public std::exception {
    mutable AOptional<std::string> mMessage;

public:
    AException() : mStacktrace(AStacktrace::capture(2)) {}
    AException(AStacktrace stacktrace) : mStacktrace(std::move(stacktrace)) {}

    AException(const AString& message) : AException() { mMessage = message.toStdString(); }

    AException(const AException& exception) = default;
    AException(AException&& exception) noexcept = default;

    AException(const AString& message, std::exception_ptr causedBy, AStacktrace stacktrace = AStacktrace::capture(2))
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
    std::exception_ptr mCausedBy;
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