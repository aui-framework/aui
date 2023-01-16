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

#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <variant>
#include <optional>

class API_AUI_CORE AStacktrace {
public:
    class Entry {
        friend class AStacktrace;
    private:
        void* mPtr;
        AOptional<AString> mFunctionName;
        AOptional<AString> mFileName;
        AOptional<unsigned> mLineNumber;

    public:
        explicit Entry(void* ptr) : mPtr(ptr), mFunctionName(std::nullopt) {}

        const AOptional<AString>& functionName() const noexcept {
            return mFunctionName;
        }

        const AOptional<AString>& fileName() const noexcept {
            return mFileName;
        }

        AOptional<unsigned> lineNumber() const noexcept {
            return mLineNumber;
        }

        void* ptr() const noexcept {
            return mPtr;
        }
    };
public:
    AStacktrace(const AStacktrace&) = default;
    AStacktrace(AStacktrace&&) noexcept = default;

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

