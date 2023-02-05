// AUI Framework - Declarative UI toolkit for modern C++20
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

#include <string>
#include <iostream>
#include "AUI/Core.h"
#include <AUI/Common/ASet.h>
#include <optional>
#include <AUI/Common/AOptional.h>

class API_AUI_CORE AStringVector;
class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AByteBufferView;

/**
 * @brief Represents a wide char string.
 * @ingroup core
 */
class API_AUI_CORE AString: std::wstring
{
private:
    friend struct std::hash<AString>;
    using super = std::wstring;

public:

    using iterator = super::iterator;
    using const_iterator = super::const_iterator;
    using reverse_iterator = super::reverse_iterator;
    using const_reverse_iterator = super::const_reverse_iterator;
    auto constexpr static NPOS = super::npos;


    AString(AString&& other) noexcept
            : std::wstring(static_cast<basic_string&&>(other))
    {
    }

    /**
     * @param other utf8 string
     */
    AString(const basic_string& other) noexcept
            : basic_string<wchar_t>(other)
    {
    }

    /**
     * @param utf8 utf8 string
     */
    AString(const std::string& utf8) noexcept;

    AString(const AString& other) noexcept
            : super(other.c_str())
    {
    }

    AString(const basic_string& _Right, const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Right, _Al)
    {
    }

    template <class Iterator>
    AString(Iterator first, Iterator last) noexcept : super(first, last) {}

    AString() noexcept
    {
    }

    AString(wchar_t c) noexcept : super(&c, &c + 1)
    {

    }

    /**
     * @param utf8 utf8 string
     */
    AString(const char* utf8) noexcept;

    /**
     * @param utf8 utf8 string
     */
    AString(std::string_view utf8) noexcept;

    explicit AString(const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Al)
    {
    }

    AString(const basic_string& _Right, size_type _Roff, const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Right, _Roff, _Al)
    {
    }

    AString(const basic_string& _Right, size_type _Roff, size_type _Count, const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Right, _Roff, _Count, _Al)
    {
    }

    AString(const wchar_t* _Ptr, size_type _Count) noexcept
            : basic_string<wchar_t>(_Ptr, _Count)
    {
    }

    AString(const wchar_t* _Ptr, size_type _Count, const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Ptr, _Count, _Al)
    {
    }

    AString(const wchar_t* _Ptr) noexcept
            : basic_string<wchar_t>(_Ptr)
    {
    }

    AString(const wchar_t* _Ptr, const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Ptr, _Al)
    {
    }

    AString(size_type _Count, wchar_t _Ch) noexcept
            : basic_string<wchar_t>(_Count, _Ch)
    {
    }

    AString(size_type _Count, wchar_t _Ch, const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Count, _Ch, _Al)
    {
    }

    AString(basic_string&& _Right) noexcept
            : basic_string<wchar_t>(_Right)
    {
    }

    AString(basic_string&& _Right, const std::allocator<wchar_t>& _Al) noexcept
            : basic_string<wchar_t>(_Right, _Al)
    {
    }

    AString(std::initializer_list<wchar_t> _Ilist) noexcept
            : basic_string<wchar_t>(_Ilist)
    {
    }

    ~AString() = default;


    void push_back(wchar_t c) noexcept
    {
        super::push_back(c);
    }
    void pop_back() noexcept
    {
        super::pop_back();
    }

    AString uppercase() const;
    AString lowercase() const;

    bool startsWith(const AString& other) const noexcept
    {
        return rfind(other, 0) == 0;
    }
    bool startsWith(wchar_t c) const noexcept
    {
        return rfind(c, 0) == 0;
    }
    bool endsWith(const AString& other) const noexcept
    {
        if (length() < other.length())
        {
            return false;
        }
        size_t offset = length() - other.length();
        return super::find(other, offset) == offset;
    }
    bool endsWith(wchar_t c) const noexcept
    {
        size_t offset = length() - 1;
        return super::find(c, offset) == offset;
    }

    AStringVector split(wchar_t c) const noexcept;

    size_type find(char c, size_type offset = 0) const noexcept
    {
        return super::find(c, offset);
    }
    size_type find(wchar_t c, size_type offset = 0) const noexcept
    {
        return super::find(c, offset);
    }
    size_type find(const AString& str, size_type offset = 0) const noexcept
    {
        return super::find(str, offset);
    }
    size_type rfind(char c, size_type offset = NPOS) const noexcept
    {
        return super::rfind(c, offset);
    }
    size_type rfind(wchar_t c, size_type offset = NPOS) const noexcept
    {
        return super::rfind(c, offset);
    }
    size_type rfind(const AString& str, size_type offset = NPOS) const noexcept
    {
        return super::rfind(str, offset);
    }
    size_type length() const noexcept
    {
        return super::length();
    }
    AString trimLeft(wchar_t symbol = ' ') const noexcept;
    AString trimRight(wchar_t symbol = ' ') const noexcept;

    AString trim(wchar_t symbol = ' ') const noexcept
    {
        return trimRight(symbol).trimLeft(symbol);
    }

    void reserve(size_t s)
    {
        super::reserve(s);
    }
    void resize(size_t s)
    {
        super::resize(s);
    }

    AString restrictLength(size_t s, const AString& stringAtEnd = "...") const;

    wchar_t* data() noexcept
    {
        return super::data();
    }

    const wchar_t* data() const noexcept
    {
        return super::data();
    }
    AString& replaceAll(const AString& from, const AString& to);
    [[nodiscard]] AString replacedAll(const AString& from, const AString& to) const;
    [[nodiscard]] inline AString replacedAll(wchar_t from, wchar_t to) const noexcept {
        AString copy;
        copy.reserve(length() + 10);
        for (auto c : *this) {
            if (c == from) {
                copy << to;
            } else {
                copy << c;
            }
        }
        return copy;
    }
    [[nodiscard]] inline AString replacedAll(const ASet<wchar_t>& from, wchar_t to) const noexcept {
        AString copy;
        copy.reserve(length() + 10);
        for (auto c : *this) {
            if (from.contains(c)) {
                copy << to;
            } else {
                copy << c;
            }
        }
        return copy;
    }
    AString& replaceAll(wchar_t from, wchar_t to) noexcept;


    /**
     * @brief Converts the string to a float number.
     * @return The string converted to a float number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<float> toFloat() const noexcept;

    /**
     * @brief Converts the string to a double number.
     * @return The string converted to a double number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<double> toDouble() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<int> toInt() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<int64_t> toLongInt() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<unsigned> toUInt() const noexcept;

    /**
     * @brief Converts the string to boolean value.
     * @return If the string equals to "true", true returned, false otherwise.
     */
    [[nodiscard]]
    bool toBool() const noexcept {
        return *this == "true";
    }

    [[nodiscard]]
    bool contains(wchar_t c) const noexcept
    {
        return find(c) != npos;
    }
    [[nodiscard]]
    bool contains(const AString& other) const noexcept
    {
        return find(other) != npos;
    }

    static AString fromLatin1(const AByteBuffer& buffer);
    static AString fromUtf8(const AByteBufferView& buffer);
    static AString fromUtf8(const char* buffer, size_t length);
    static AString fromLatin1(const char* buffer);

    static AString numberHex(int i) noexcept;

    template<typename T, std::enable_if_t<std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int> = 0>
    static AString number(T i) noexcept {
        if constexpr (std::is_same_v<bool, std::decay_t<T>>) {
            if (i)
                return "true";
            return "false";
        } else {
            auto v = std::to_wstring(i);
            if constexpr (std::is_floating_point_v<T>) {
                // remove trailing zeros
                v.erase(v.find_last_not_of('0') + 1, std::wstring::npos);
                v.erase(v.find_last_not_of('.') + 1, std::wstring::npos);
            }
            return v;
        }
    }
    int toNumberDec() const noexcept;
    int toNumberHex() const noexcept;

    /**
     * @return utf8-encoded std::string.
     */
    std::string toStdString() const noexcept;

    void resizeToNullTerminator();

    iterator erase(const_iterator begin, const_iterator end) noexcept
    {
        return super::erase(begin, end);
    }
    iterator erase(const_iterator begin) noexcept
    {
        return super::erase(begin);
    }

    AString& erase(size_type offset) noexcept
    {
        super::erase(offset);
        return *this;
    }
    AString& erase(size_type offset, size_type count) noexcept
    {
        super::erase(offset, count);
        return *this;
    }

    AByteBuffer toUtf8() const noexcept;

    void removeAt(unsigned at) noexcept
    {
        assert(at <= length());
        erase(begin() + at);
    }
    AString excessSpacesRemoved() const noexcept;

    iterator insert(size_type at, wchar_t c) noexcept
    {
        assert(at <= length());
        return super::insert(begin() + at, 1, c);
    }
    iterator insert(size_type at, const AString& c) noexcept
    {
        assert(at <= length());
        return super::insert(begin() + at, c.begin(), c.end());
    }

    template<typename Iterator>
    iterator insert(const_iterator at, Iterator begin, Iterator end) noexcept
    {
        assert(std::distance(super::cbegin(), at) <= length());
        return super::insert(at, begin, end);
    }

    AString& operator<<(char c) noexcept
    {
        append(1, c);
        return *this;
    }
    AString& operator<<(wchar_t c) noexcept
    {
        append(1, c);
        return *this;
    }

    inline ::AString& operator+=(const AString& str) noexcept
    {
        append(str);
        return *this;
    }
    inline ::AString& operator+=(const char* str) noexcept
    {
        *this += AString(str);
        return *this;
    }

    [[nodiscard]] bool empty() const noexcept {
        return super::empty();
    }
    [[nodiscard]] size_type size() const noexcept {
        return super::size();
    }
    wchar_t operator[](size_type index) const
    {
        return super::at(index);
    }
    wchar_t& operator[](size_type index)
    {
        return super::at(index);
    }
    bool operator<(const AString& other) const noexcept
    {
        return compare(other) < 0;
    }

    void clear() noexcept
    {
        super::clear();
    }

    wchar_t& front() noexcept
    {
        return super::front();
    }
    wchar_t& back() noexcept
    {
        return super::back();
    }
    const wchar_t& front() const noexcept
    {
        return super::front();
    }
    const wchar_t& back() const noexcept
    {
        return super::back();
    }
    wchar_t& first() noexcept
    {
        return super::front();
    }
    wchar_t& last() noexcept
    {
        return super::back();
    }
    const wchar_t& first() const noexcept
    {
        return super::front();
    }
    const wchar_t& last() const noexcept
    {
        return super::back();
    }

    const wchar_t* c_str() const
    {
        return super::c_str();
    }

    iterator begin() noexcept
    {
        return super::begin();
    }
    iterator end() noexcept
    {
        return super::end();
    }

    const_iterator begin() const noexcept
    {
        return super::begin();
    }
    const_iterator end() const noexcept
    {
        return super::end();
    }

    reverse_iterator rbegin() noexcept
    {
        return super::rbegin();
    }
    reverse_iterator rend() noexcept
    {
        return super::rend();
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return super::rbegin();
    }
    const_reverse_iterator rend() const noexcept
    {
        return super::rend();
    }

    AString& append(const AString& s) noexcept
    {
        super::append(s);
        return *this;
    }

    AString& append(size_t count, wchar_t ch) noexcept
    {
        super::append(count, ch);
        return *this;
    }

    const AString& operator=(const AString& value) noexcept
    {
        super::operator=(value);
        return *this;
    }

    const AString& operator=(AString&& value) noexcept
    {
        super::operator=(value);
        return *this;
    }

    bool operator==(const AString& other) const noexcept
    {
        return wcscmp(c_str(), other.c_str()) == 0;
    }
    bool operator==(const wchar_t* other) const noexcept
    {
        return wcscmp(c_str(), other) == 0;
    }
    bool operator==(const char* other) const noexcept
    {
        return *this == AString(other);
    }

    bool operator!=(const AString& other) const noexcept
    {
        return wcscmp(c_str(), other.c_str()) != 0;
    }
    bool operator!=(const wchar_t* other) const noexcept
    {
        return wcscmp(c_str(), other) != 0;
    }
    bool operator!=(const char* other) const noexcept
    {
        return *this != AString(other);
    }

    template<typename... Args>
    inline AString format(Args&&... args) const;

    AString processEscapes() const;

    AString& removeAll(wchar_t c) noexcept {
        erase(std::remove(begin(), end(), c));
        return *this;
    }

    [[nodiscard]]
    AString substr(std::size_t offset, std::size_t count = npos) const {
        return super::substr(offset, count);
    }

private:
    /**
     * @brief Converts the string to integer values. Used in AString::toInt, AString::toUInt, etc.
     */
    template<typename T>
    AOptional<T> toNumberImpl() const noexcept;
};

inline AString operator+(const AString& l, const AString& r) noexcept
{
    auto x = l;
    x.append(r);
    return x;
}
inline AString operator+(const AString& l, wchar_t r) noexcept
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

inline AString operator"" _as(const char* str, size_t len)
{
    return {str};
}

inline std::ostream& operator<<(std::ostream& o, const AString& s)
{
    o << s.toStdString();
    return o;
}
namespace std
{
    template<>
    struct hash<AString>
    {
        size_t operator()(const AString& t) const
        {
            return hash<std::wstring>()(t);
        }
    };
}

// gtest printer for AString
inline void PrintTo(const AString& s, std::ostream* stream) {
    *stream << s.toStdString();
}
