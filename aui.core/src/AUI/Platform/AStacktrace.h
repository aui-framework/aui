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
        std::optional<AString> mFunctionName;
        std::optional<AString> mFileName;
        std::optional<unsigned> mLineNumber;

        void resolveIfNeeded() const noexcept;

    public:
        explicit Entry(void* ptr) : mPtr(ptr), mFunctionName(std::nullopt) {}

        const std::optional<AString>& functionName() const noexcept {
            return mFunctionName;
        }

        const std::optional<AString>& fileName() const noexcept {
            return mFileName;
        }

        std::optional<unsigned> lineNumber() const noexcept {
            return mLineNumber;
        }

        void* ptr() const noexcept {
            return mPtr;
        }
    };

private:
    mutable AVector<Entry> mEntries;
    mutable bool mSymbolNamesResolved = false;

    explicit AStacktrace(AVector<Entry> entries) : mEntries(std::move(entries)) {}
public:

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

    static AStacktrace capture(unsigned skipFrames = 0) noexcept;
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

