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

    using super::super;

    AString(const AString& other) : super(other) {}

    AString(AString&& other) noexcept : super(std::move(other)) {}

    AString(std::span<const std::byte> bytes, AStringEncoding encoding);

    explicit AString(AStringView view);

    AString(super&& other) : super(std::move(other)) {}

    AString(AChar c);

    ~AString() = default;

    using super::push_back;

    void push_back(AChar c) noexcept;

    AByteBuffer getBytes(AStringEncoding encoding) const;

    /// Compatibility method
    std::string toStdString() const {
        return *this;
    }

    /// Returns the number of bytes in the UTF-8 encoded string
    size_type sizeBytes() const noexcept {
        return size();
    }

    /// Returns the number of Unicode characters in the string
    size_type length() const noexcept;

    AOptional<int32_t> toInt() const noexcept;

    AOptional<int64_t> toLong() const noexcept;

    AOptional<uint32_t> toUInt() const noexcept;

    AOptional<uint64_t> toULong() const noexcept;

    AOptional<float> toFloat() const noexcept;

    AOptional<double> toDouble() const noexcept;

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

};
