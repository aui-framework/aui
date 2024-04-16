// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <variant>
#include <optional>


/**
 * @brief Stacktrace consisting of a collection of stack function frames.
 * @ingroup core
 * @details
 * The first element of the collection represents the top of the stack, which is the last function invocation.
 * Typically, this is the point at which this stacktrace was created (unless skipFrames is zero). The last element of
 * the collection represents the bottom of the stack (typically entrypoint).
 */
class API_AUI_CORE AStacktrace {
public:
    /**
     * @brief Stacktrace entry.
     */
    class Entry {
        friend class AStacktrace;
    private:
        void* mPtr;
        AOptional<AString> mFunctionName;
        AOptional<AString> mFileName;
        AOptional<unsigned> mLineNumber;

    public:
        explicit Entry(void* ptr) : mPtr(ptr), mFunctionName(std::nullopt) {}

        /**
         * @see AStacktrace::resolveSymbolsIfNeeded()
         */
        const AOptional<AString>& functionName() const noexcept {
            return mFunctionName;
        }

        /**
         * @see AStacktrace::resolveSymbolsIfNeeded()
         */
        const AOptional<AString>& fileName() const noexcept {
            return mFileName;
        }

        /**
         * @see AStacktrace::resolveSymbolsIfNeeded()
         */
        AOptional<unsigned> lineNumber() const noexcept {
            return mLineNumber;
        }

        void* ptr() const noexcept {
            return mPtr;
        }
    };

    using iterator = AVector<Entry>::iterator;

    AStacktrace(aui::range<iterator> range): mEntries(range.begin(), range.end()) {}

    /**
     * @brief Tries to find the function pointer in stacktrace and strips stacktrace until function frame.
     * @param pFunction pointer to function.
     * @param maxAllowedOffsetInBytes max offset error in bytes.
     * @return itself (modified)
     * @details
     * If stripBeforeFunctionCall fails to find pFunction, it leaves AStacktrace unmodified and returns itself.
     *
     * stripBeforeFunctionCall strips the end of stacktrace, excluding the passed function.
     *
     * maxAllowedOffsetInBytes is intended to find stack frame, which is not in exact location of function pointer.
     */
    aui::range<iterator> stripBeforeFunctionCall(void* pFunction, int maxAllowedOffsetInBytes = 50);

    AStacktrace(const AStacktrace&) = default;
    AStacktrace(AStacktrace&&) noexcept = default;

    /**
     * @brief Invokes function name resolution with function pointers.
     * @details
     * Must be called before using AStacktrace::Entry::functionName() and similar functions.
     */
    void resolveSymbolsIfNeeded() const noexcept;

    [[nodiscard]]
    const AVector<Entry>& entries() const noexcept {
        return mEntries;
    }

    [[nodiscard]]
    auto begin() const noexcept {
        return mEntries.begin();
    }

    [[nodiscard]]
    auto end() const noexcept {
        return mEntries.end();
    }

    [[nodiscard]]
    auto rbegin() const noexcept {
        return mEntries.rbegin();
    }

    [[nodiscard]]
    auto rend() const noexcept {
        return mEntries.rend();
    }

    /**
     * @brief Creates stacktrace of the current thread.
     * @param skipFrames number of frames to skip.
     * @param maxFrames max number of frames.
     * @note A call to <code>AStacktrace::capture</code> is always skipped in the stacktrace.
     * @return
     */
    static AStacktrace capture(unsigned skipFrames = 0, unsigned maxFrames = 128) noexcept;

private:
    mutable AVector<Entry> mEntries;
    mutable bool mSymbolNamesResolved = false;

    explicit AStacktrace(AVector<Entry> entries) : mEntries(std::move(entries)) {}
};

inline std::ostream& operator<<(std::ostream& o, const AStacktrace& stacktrace) noexcept {
    stacktrace.resolveSymbolsIfNeeded();
    for (const auto& entry : stacktrace) {
        o << " - at " << entry.ptr();
        if (auto name = entry.functionName()) {
            o << " " << *name;
        }
        o << "(";

        if (auto filename = entry.fileName()) {
            o << *filename;
        } else {
            o << "?";
        }
        o << ":";
        if (auto line = entry.lineNumber()) {
            o << *line;
        } else {
            o << "?";
        }

        o << ")" << std::endl;
    }
    return o;
}

