/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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