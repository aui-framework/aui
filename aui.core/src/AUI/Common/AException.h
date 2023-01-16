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
class API_AUI_CORE AException: public std::exception
{
    mutable AOptional<std::string> mMessage;
public:
	AException(): mStacktrace(AStacktrace::capture(2)) {

    }

	AException(const AString& message)
		: AException()
	{
        mMessage = message.toStdString();
	}

	AException(const AString& message, std::exception_ptr causedBy, AStacktrace stacktrace = AStacktrace::capture(2))
		: mCausedBy(std::move(causedBy)),
          mStacktrace(std::move(stacktrace))
	{
        mMessage = message.toStdString();
	}

    virtual AString getMessage() const noexcept {
        return mMessage ? mMessage->c_str() : "<no message>";
    }

	virtual ~AException() noexcept = default;

    const char* what() const noexcept override {
        if (!mMessage) mMessage = getMessage().toStdString();
        return mMessage->c_str();
    }

    const AStacktrace& stacktrace() const noexcept {
        return mStacktrace;
    }

    [[nodiscard]]
    const std::exception_ptr& causedBy() const noexcept {
        return mCausedBy;
    }

private:
    AStacktrace mStacktrace;
    std::exception_ptr mCausedBy;

};

inline std::ostream& operator<<(std::ostream& o, const AException& e) noexcept {
    o << "("
      << AReflect::name(&e)
      << ") "
      << e.getMessage()
      << std::endl
      << e.stacktrace();
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