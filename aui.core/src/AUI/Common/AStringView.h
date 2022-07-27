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

#include <string>
#include <iostream>
#include "AUI/Core.h"
#include "AChar.h"
#include <AUI/Common/ASet.h>
#include <AUI/Common/AStringView.h>
#include <optional>

class API_AUI_CORE AStringVector;
class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AByteBufferView;
class API_AUI_CORE AString;

/**
 * @brief Represents an unmodifiable wide char string representation, which does not own it's contents.
 * @ingroup core
 * @details
 * Main goal of the string view is reduce unwanted copies and heap memory usage by borrowing the string contents. Most
 * of the non-modifying string operations like substr(), trim(), toInt(), toStdString() don't require extra heap
 * allocation and copy.
 *
 * Using regular AString as a function argument causes unwanted heap allocation copy when passing string literal.
 * There's no such problem when using AStringView.
 *
 * @note
 * AStringView does not own the string it is refers to. If the referred string is deleted, AStringView refers to the
 * invalid memory.
 */
class API_AUI_CORE AStringView
{
public:
    struct iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = const AChar;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator(const AChar* pointer) : mPointer(pointer) {}

        [[nodiscard]]
        bool operator==(iterator rhs) const noexcept {
            return mPointer == rhs.mPointer;
        }

        [[nodiscard]]
        bool operator!=(iterator rhs) const noexcept {
            return mPointer != rhs.mPointer;
        }

        iterator operator++() const noexcept {
            return { mPointer + 1 };
        }
        iterator& operator++() noexcept {
            mPointer++;
            return *this;
        }

        iterator operator--() const noexcept {
            return { mPointer - 1 };
        }
        iterator& operator--() noexcept {
            mPointer--;
            return *this;
        }

        [[nodiscard]]
        iterator operator+(std::size_t i) const noexcept {
            return { mPointer + i };
        }

        [[nodiscard]]
        iterator operator-(std::size_t i) const noexcept {
            return { mPointer - i };
        }

        [[nodiscard]]
        std::size_t operator-(iterator rhs) const noexcept {
            return mPointer - rhs.mPointer;
        }

        const AChar& operator*() const noexcept {
            return *mPointer;
        }

    private:
        const AChar* mPointer;
    };

    using reverse_iterator = std::reverse_iterator<iterator>;

    AStringView(const AChar* data, std::size_t length) noexcept: mData(data), mLength(length) {}
    AStringView(const wchar_t* data, std::size_t length) noexcept: mData(reinterpret_cast<const AChar *>(data)), mLength(length) {}

    template<std::size_t LENGTH>
    constexpr AStringView(const wchar_t (&data)[LENGTH]) noexcept: mData(reinterpret_cast<const AChar *>(data)), mLength(LENGTH) { // for string literals

    }

    [[nodiscard]]
    iterator begin() const noexcept {
        return { mData };
    }

    [[nodiscard]]
    iterator end() const noexcept {
        return { mData + mLength };
    }

    [[nodiscard]]
    reverse_iterator rbegin() const noexcept {
        return std::make_reverse_iterator(end());
    }

    [[nodiscard]]
    reverse_iterator rend() const noexcept {
        return std::make_reverse_iterator(begin());
    }

    /**
     * @brief Returns the string transformed to uppercase.
     */
    [[nodiscard]]
    AString uppercased() const;

    /**
     * @brief Returns the string transformed to lowercase.
     */
    [[nodiscard]]
    AString lowercased() const;

    /**
     * @brief Returns true if the string starts with other; otherwise returns false.
     */
    [[nodiscard]]
    bool startsWith(AStringView other) const noexcept
    {
        return rfind(other, 0) == 0;
    }

    /**
     * @brief Returns true if the string starts with c; otherwise returns false.
     */
    bool startsWith(AChar c) const noexcept
    {
        return rfind(c, 0) == 0;
    }

    /**
     * @brief Returns true if the string ends with other; otherwise returns false.
     */
    bool endsWith(AStringView other) const noexcept
    {
        if (length() < other.length())
        {
            return false;
        }
        size_t offset = length() - other.length();
        return find(other, offset) == offset;
    }

    /**
     * @brief Returns true if the string ends with c; otherwise returns false.
     */
    bool endsWith(AChar c) const noexcept
    {
        size_t offset = length() - 1;
        return find(c, offset) == offset;
    }

    /**
     * @brief Splits the string into substrings wherever separator occurrences.
     * @param separator separator
     * @details
     * If separator never occurrences in the string, one element returned containing the string.
     */
    AStringVector split(AChar separator) const noexcept;


    /**
     * @brief Finds for the first occurrence of c in the string.
     * @param c the character to find
     * @param offset initial offset
     * @return Index of the first occurrence of the character c, -1 if not found.
     */
    [[nodiscard]]
    int find(AChar c, std::size_t offset = 0) const noexcept
    {
        auto v = std::find(begin() + offset, end(), c);
        if (v == end()) return -1;
        return static_cast<int>(v - begin());
    }

    /**
     * @brief Finds for the first occurrence of str in the string.
     * @param str the string to find.
     * @param offset initial offset.
     * @return Index of the first occurrence of the substring str, -1 if not found.
     */
    [[nodiscard]]
    int find(AStringView str, std::size_t offset = 0) const noexcept;

    /**
     * @brief Finds for the last occurrence of c in the string.
     * @param c the character to find.
     * @param offset initial offset.
     * @return Index of the first occurrence of the character c, -1 if not found.
     */
    [[nodiscard]]
    int rfind(AChar c, std::size_t offset = 0) const noexcept
    {
        auto v = std::find(rbegin() + offset, rend(), c);
        if (v == rend()) return -1;
        return std::distance(v.base() + 1, begin());
    }

    /**
     * @brief Finds for the last occurrence of str in the string.
     * @param c the character to find
     * @param offset initial offset
     * @brief Returns returns index of the first occurrence of the character c, -1 if not found
     */
    int rfind(AStringView str, std::size_t offset = -1) const noexcept;

    [[nodiscard]]
    std::size_t length() const noexcept
    {
        return mLength;
    }

    /**
     * @brief Removes matched symbol (whitespace by default) from the left side of the string.
     * @param predicate Predicate which determines the symbols to remove (whitespace by default).
     */
    template<typename Predicate>
    [[nodiscard]]
    AStringView& trimLeft(Predicate&& predicate = [](AChar c) { return c.whitespace(); }) noexcept {
        auto it = std::find_if(begin(), end(), [&](AChar c) {
            return !predicate(c);
        });
        mData = &*it;
        mLength = std::distance(it, end());
        return *this;
    }

    /**
     * @brief Removes matched symbol (whitespace by default) from the right side of the string.
     * @param predicate Predicate which determines the symbols to remove (whitespace by default).
     */
    template<typename Predicate>
    AStringView& trimRight(Predicate&& predicate = [](AChar c) { return c.whitespace(); }) noexcept {
        auto it = std::find_if(rbegin(), rend(), [&](AChar c) {
            return !predicate(c);
        });
        mLength = std::distance(it.base() + 1, end());
        return *this;
    }


    /**
     * @brief Removes matched symbol (whitespace by default) from the both sides of the string.
     * @param predicate Predicate which determines the symbols to remove (whitespace by default).
     */
    template<typename Predicate>
    AStringView& trim(Predicate&& predicate = [](AChar c) { return c.whitespace(); }) noexcept
    {
        trimLeft(predicate);
        trimRight(predicate);
        return *this;
    }

    void setLength(size_t s)
    {
        mLength = s;
    }

    [[nodiscard]]
    AString restrictedLength(size_t maxCharacterCount, AStringView stringAtEnd = L"...") const;


    /**
     * @breif Returns a pointer to string data. Null terminator is not guaranteed.
     */
    [[nodiscard]]
    const AChar* data() const noexcept
    {
        return mData;
    }

    /**
     * @brief Replaces all occurrences of from with to in a copy of the string.
     * @param from the string to replace from.
     * @param to the string to replace to.
     */
    [[nodiscard]]
    AString replacedAll(AStringView from, AStringView to) const noexcept;

    /**
     * @brief Replaces all occurrences of from with to in a copy of the string.
     * @param from the char to replace from.
     * @param to the char to replace to.
     */
    [[nodiscard]] AString replacedAll(AChar from, AChar to) const noexcept;

    /**
     * @brief Replaces all occurrences of from with to in a copy of the string.
     * @param from the char set to replace from.
     * @param to the char to replace to.
     */
    [[nodiscard]] AString replacedAll(const ASet<AChar>& from, AChar to) const noexcept;

    /**
     * @brief Converts the string to a float number.
     * @brief Returns the string converted to a float number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    std::optional<float> toFloat() const noexcept;

    /**
     * @brief Converts the string to a double number.
     * @brief Returns the string converted to a double number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    std::optional<double> toDouble() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @brief Returns the string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    std::optional<int> toInt() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @brief Returns the string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    std::optional<unsigned> toUInt() const noexcept;

    /**
     * @brief Converts the string to boolean value.
     * @brief Returns if the string equals to "true", true returned, false otherwise.
     */
    [[nodiscard]]
    bool toBool() const noexcept {
        return *this == "true";
    }

    /**
     * @brief Checks whether char c appears in the string or not.
     */
    [[nodiscard]]
    bool contains(AChar c) const noexcept
    {
        return find(c) != -1;
    }

    /**
     * @brief Checks whether string str appears in the string or not.
     */
    [[nodiscard]]
    bool contains(AStringView str) const noexcept
    {
        return find(str) != -1;
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
            return std::to_wstring(i);
        }
    }
    int toNumberDec() const noexcept;
    int toNumberHex() const noexcept;

    /**
     * @brief Returns utf8-encoded std::string.
     */
    std::string toStdString() const noexcept;

    AByteBuffer toUtf8() const noexcept;

    AString excessSpacesRemoved() const noexcept;

    [[nodiscard]] bool empty() const noexcept {
        return mLength == 0;
    }

    AChar operator[](std::size_t index) const noexcept
    {
        assert(index < mLength);
        return mData[index];
    }

    [[nodiscard]]
    std::wstring_view toStdStringView() const noexcept {
        return {reinterpret_cast<const wchar_t *const>(mData), mLength };
    }

    [[nodiscard]]
    bool operator<(AStringView other) const noexcept
    {
        return toStdStringView() < other.toStdStringView();
    }


    AChar front() noexcept
    {
        return mData[0];
    }

    AChar back() noexcept
    {
        return mData[mLength];
    }

    AChar first() noexcept
    {
        return front();
    }

    AChar last() noexcept
    {
        return back();
    }

    template<typename... Args>
    inline AString format(Args&&... args) const;

    AString processEscapes() const;

    AString removedAll(AChar c) const;

    [[nodiscard]]
    AStringView substr(std::size_t offset, std::size_t count = -1) const {
        assert(offset + count <= mLength);
        return { mData + offset, count };
    }

private:
    const AChar* mData;
    std::size_t mLength;

    /**
     * @brief Converts the string to integer values. Used in AStringView::toInt, AStringView::toUInt, etc.
     */
    template<typename T>
    std::optional<T> toNumberImpl() const noexcept;
};


inline std::ostream& operator<<(std::ostream& o, AStringView s)
{
    o << s.toStdString();
    return o;
}

inline AStringView operator"" _as(const wchar_t* str, size_t len)
{
    return { str, len };
}


// gtest printer for AString
inline void PrintTo(AStringView s, std::ostream* stream) {
    *stream << s.toStdString();
}
