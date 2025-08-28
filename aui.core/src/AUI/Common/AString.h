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

#include <algorithm>
#include <string>
#include <string_view>
#include <iostream>
#include <AUI/Core.h>
#include <AUI/Traits/values.h>
#include <AUI/Common/ASet.h>
#include <AUI/Common/AChar.h>
#include <optional>
#include <span>
#include <AUI/Common/AOptional.h>
#include <fmt/core.h>

class API_AUI_CORE AStringVector;
class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AByteBufferView;

enum class AStringEncoding : uint8_t {
    UTF8 = 0,
    UTF16 = 1,
    UTF32 = 2,
    LATIN1 = 3,
};

class AStringView;

/**
 * @brief Represents a Unicode character string.
 * @ingroup core
 * @details
 * AString stores a string of 16-bit chars, where each char corresponds to one UTF-16 code unit. Unicode characters with
 * code values above 65535 are stored using two consecutive chars.
 *
 * Unicode is an international standard that supports most of the writing systems in use today.
 */
class API_AUI_CORE AString: public std::string
{
private:
    friend struct std::hash<AString>;
    using super = std::string;

public:

    using iterator = super::iterator;
    using value_type = super::value_type;
    using const_iterator = super::const_iterator;
    using reverse_iterator = super::reverse_iterator;
    using const_reverse_iterator = super::const_reverse_iterator;
    auto constexpr static NPOS = super::npos;

    static AString numberHex(int i);

    template<typename T, std::enable_if_t<std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int> = 0>
    static AString number(T i) noexcept {
        if constexpr (std::is_same_v<bool, std::decay_t<T>>) {
            if (i) return "true";
            return "false";
        } else {
            auto v = std::to_string(i);
            if constexpr (std::is_floating_point_v<T>) {
                // remove trailing zeros
                v.erase(v.find_last_not_of('0') + 1, std::u16string::npos);
                v.erase(v.find_last_not_of('.') + 1, std::u16string::npos);
            }
            return v;
        }
    }

    static constexpr auto TO_NUMBER_BASE_BIN = 2;
    static constexpr auto TO_NUMBER_BASE_OCT = 8;
    static constexpr auto TO_NUMBER_BASE_DEC = 10;
    static constexpr auto TO_NUMBER_BASE_HEX = 16;

    template<class T>
    constexpr static size_type strLength(const T* str) noexcept {
        if (str == nullptr) {
            return 0;
        }

        size_type length = 0;
        while (str[length] != T()) {
            ++length;
        }
        return length;
    }

    using super::super;

    AString(const AString& other) : super(other) {}

    AString(AString&& other) noexcept : super(std::move(other)) {}

    AString(std::span<const std::byte> bytes, AStringEncoding encoding);

    AString(const char16_t* utf16_bytes, size_type length);

    AString(const char16_t* utf16_bytes) : AString(utf16_bytes, strLength(utf16_bytes)) {}

    AString(const char32_t* utf32_bytes, size_type length);

    AString(const char32_t* utf32_bytes) : AString(utf32_bytes, strLength(utf32_bytes)) {}

    explicit AString(AStringView view);

    constexpr AString(super&& other) : super(std::move(other)) {}

    AString(AChar c);

    ~AString() = default;

    using super::push_back;

    void push_back(AChar c) noexcept;

    AByteBuffer getBytes(AStringEncoding encoding) const;

    /// Compatibility method
    std::string toStdString() const {
        return *this;
    }

    operator AStringView() const noexcept;

    /// Returns the number of bytes in the UTF-8 encoded string
    constexpr size_type sizeBytes() const noexcept {
        return size();
    }

    /// Returns the number of Unicode characters in the string
    size_type length() const noexcept;

    constexpr AString substr(size_type pos = 0, size_type n = npos) const {
        return AString(super::substr(pos, n));
    }

    AString& operator=(const AString& other) {
        std::string& ul = *this;
        ul = other;
        return *this;
    }

    AString& operator=(AString&& other) noexcept {
        std::string& ul = *this;
        ul = std::move(other);
        return *this;
    }

    using super::append;

    AString& append(char c);

    AString& append(AChar c);

    AString uppercase() const;

    AString lowercase() const;

    AStringVector split(AChar c) const;

    bool startsWith(AStringView prefix) const noexcept;

    bool endsWith(AStringView suffix) const noexcept;

    bool toBool() const {
        return sizeBytes() == 4 && lowercase() == "true";
    }

    AOptional<int32_t> toInt() const noexcept;

    AOptional<int64_t> toLong() const noexcept;

    AOptional<uint32_t> toUInt() const noexcept;

    AOptional<uint64_t> toULong() const noexcept;

    AOptional<float> toFloat() const noexcept;

    AOptional<double> toDouble() const noexcept;

    /**
     * @brief Returns the string converted to an int using base. Returns std::nullopt if the conversion fails.
     * @sa toNumberOrException
     */
    AOptional<int> toNumber(aui::ranged_number<int, 2, 36> base) const noexcept;

    /**
     * @brief Returns the string converted to an int using base. Throws an exception if the conversion fails.
     * @sa toNumber
     */
    int toNumberOrException(aui::ranged_number<int, 2, 36> base = TO_NUMBER_BASE_DEC) const {
        return toNumber(base).valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    template<typename... Args>
    AString format(Args&&... args) const;

private:
    size_type size() const noexcept {
        return super::size();
    }
};

inline AString operator+(const AString& l, const AString& r) noexcept
{
    return static_cast<const std::string&>(l) + r;
}
inline AString operator+(const AString& l, char r) noexcept
{
    auto x = l;
    x.append(r);
    return x;
}
inline AString operator+(const AString& l, AChar r) noexcept
{
    auto x = l;
    x.append(r);
    return x;
}
inline AString operator+(const AString& one, const char* other) noexcept
{
    return one + AString(other);
}

inline AString operator+(const char* other, const AString& one) noexcept {
    return AString(other) + one;
}

inline AString operator+(char lhs, const AString& cs) noexcept
{
    AString s(lhs);
    s += cs;
    return s;
}

inline AString operator""_as(const char* str, size_t len)
{
    return {str};
}

inline std::ostream& operator<<(std::ostream& o, const AString& s)
{
    o << s.toStdString();
    return o;
}

template<>
struct std::hash<AString>
{
    size_t operator()(const AString& t) const noexcept
    {
        return std::hash<std::string>()(t);
    }
};

#if defined(FMT_VERSION) && (FMT_VERSION < 100000)
template <> struct fmt::detail::is_string<AString>: std::false_type {};
#endif

template <> struct fmt::formatter<AString>: fmt::formatter<std::string> {
    auto format(const AString& s, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(s.toStdString(), ctx);
    }
};


// gtest printer for AString
inline void PrintTo(const AString& s, std::ostream* stream) {
    *stream << s.toStdString();
}

class AStringView: public std::string_view {
private:
    using super = std::string_view;

public:

    using super::super;

    constexpr AStringView(std::string_view str) noexcept : super(str) {}

    bool startsWith(AStringView prefix) const noexcept {
        if (prefix.size() > size()) {
            return false;
        }
        return substr(0, prefix.size()) == prefix;
    }

    bool endsWith(AStringView suffix) const noexcept {
        if (suffix.size() > size()) {
            return false;
        }
        return substr(size() - suffix.size()) == suffix;
    }

};

inline AString::AString(AStringView view) : super(static_cast<std::string_view>(view)) {}

inline bool AString::startsWith(AStringView prefix) const noexcept {
    return static_cast<AStringView>(*this).startsWith(prefix);
}

inline bool AString::endsWith(AStringView suffix) const noexcept {
    return static_cast<AStringView>(*this).endsWith(suffix);
}
