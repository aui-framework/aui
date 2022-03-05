/**
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
#include <optional>
#include "AUI/Core.h"
#include <AUI/Common/ASet.h>

class API_AUI_CORE AStringVector;
class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AString;

/**
 * @brief Not owning representation of the string which provides full string functionality set.
 * @note Pass <code>AStringView</code> only by value - copy is cheap and passing by value will allow compiler to
 *       store the value in registers instead of memory.
 *
 * <p>
 * Intended to use as a function argument that accepts both const char* and <a href="AString">AString</a> without any
 * overhead. Also it can be used to efficiently work with a slice of the string (i.e. substr).
 * </p>
 * <p>
 * Use this class instead of <a href="AString">AString</a> in the following cases:
 * <ul>
 *   <li>The referenced string data will not be released until <code>AStringView</code> destruction.</li>
 *   <li>You are not going to modify contents of the string.</li>
 *   <li>You are not going to use <code>c_str()</code> function.</li>
 *   <li>You want to point to the slice of <code>const char*</code> or <a href="AString">AString</a>.</li>
 * </ul>
 * </p>
 */
class API_AUI_CORE AStringView {
private:
    std::string_view mImpl;
public:
    constexpr static inline auto npos = std::string_view::npos;

    AStringView() = default;
    AStringView(const char* str): mImpl(str) {}
    AStringView(const char* str, std::size_t len): mImpl(str, len) {}
    AStringView(const AString& str);


    [[nodiscard]]
    static AStringView fromBuffer(const AByteBuffer& buffer);

    /**
     * Creates <a href="AString">AString</a> from <code>AStringView</code>.
     */
    [[nodiscard]]
    AString str() const noexcept;

    [[nodiscard]]
    int compare(AStringView other) {
        return mImpl.compare(other.mImpl);
    }

    /**
     * @return std string view
     */
    [[nodiscard]]
    std::string_view std() const noexcept {
        return mImpl;
    }


    /**
     * @return length of the stored string.
     * @note use <a href="empty">empty</a> instead of <code>length() == 0</code>.
     */
    [[nodiscard]]
    std::size_t length() const noexcept {
        return mImpl.length();
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return mImpl.empty();
    }

    [[nodiscard]]
    auto begin() noexcept {
        return mImpl.begin();
    }

    [[nodiscard]]
    auto begin() const noexcept {
        return mImpl.begin();
    }

    [[nodiscard]]
    auto end() noexcept {
        return mImpl.end();
    }

    [[nodiscard]]
    auto end() const noexcept {
        return mImpl.end();
    }

    [[nodiscard]]
    auto rbegin() noexcept {
        return mImpl.rbegin();
    }

    [[nodiscard]]
    auto rbegin() const noexcept {
        return mImpl.rbegin();
    }

    [[nodiscard]]
    auto rend() noexcept {
        return mImpl.rend();
    }

    [[nodiscard]]
    auto rend() const noexcept {
        return mImpl.rend();
    }

    [[nodiscard]]
    std::size_t find(char c, std::size_t offset = 0) const noexcept {
        return mImpl.find(c, offset);
    }

    [[nodiscard]]
    std::size_t find(AStringView str, std::size_t offset = 0) const noexcept {
        return mImpl.find(str.mImpl, offset);
    }
    [[nodiscard]]
    std::size_t rfind(AStringView str, std::size_t offset = std::string_view::npos) const noexcept {
        return mImpl.rfind(str.mImpl, offset);
    }

    [[nodiscard]]
    bool startsWith(AStringView prefix) const noexcept {
        if (length() < prefix.length()) {
            return false;
        }
        return substr(0, prefix.length()) == prefix;
    }

    [[nodiscard]]
    bool endsWith(AStringView suffix) const noexcept {
        if (length() < suffix.length()) {
            return false;
        }
        return substr(length() - suffix.length()) == suffix;
    }

    [[nodiscard]]
    char first() const {
        return *begin();
    }

    [[nodiscard]]
    char last() const {
        return *(end() - 1);
    }

    [[nodiscard]]
    bool startsWith(char c) const noexcept {
        if (empty()) return false;
        return first() == c;
    }

    [[nodiscard]]
    bool endsWith(char c) const noexcept {
        return last() == c;
    }

    [[nodiscard]]
    AStringView trimLeft(char symbol = ' ') const noexcept {
        for (auto i = begin(); i != end(); ++i) {
            if (*i != symbol) {
                return {&*i, std::size_t(end() - i)};
            }
        }
        return {};
    }

    /**
     * @return string data pointer.
     * @note this function DOES NOT guarantee that the string will be null-terminated.
     */
    [[nodiscard]]
    const char* data() const noexcept {
        return mImpl.data();
    }

    [[nodiscard]]
    AStringView trimRight(char symbol = ' ') const noexcept {

        for (auto i = rbegin(); i != rend(); ++i) {
            if (*i != symbol) {
                return {data(), std::size_t(begin() - i.base())};
            }
        }
        return {};
    }

    [[nodiscard]]
    AStringView trim(char symbol = ' ') const noexcept {
        return trimRight(symbol).trimLeft(symbol);
    }

    [[nodiscard]]
    AStringVector split(char symbol = ' ') const;

    /**
     * @return contents copy but in uppercase.
     */
    [[nodiscard]]
    AString uppercased() const;

    /**
     * @return contents copy but in lowercase.
     */
    [[nodiscard]]
    AString lowercased() const;

    AString replacedAll(char from, char to) const noexcept;

    [[nodiscard]]
    AStringView substr(std::size_t beginning, std::size_t count = npos) const noexcept {
        assert(("bad substring beginning", beginning <= length()));
        if (count == npos) {
            count = length() - beginning;
        } else {
            assert(("bad substring ending", beginning + count <= length()));
        }
        return {data() + beginning, count};
    }

    [[nodiscard]]
    AString restrictLength(size_t s, AStringView stringAtEnd = "...") const;

    [[nodiscard]]
    AString processEscapes() const;


    /**
     * @return converts utf8 string to utf16.
     */
    [[nodiscard]]
    std::wstring toUtf16() const;

    template<typename... Args>
    AString format(Args&&... args) const;

    [[nodiscard]]
    char operator[](std::size_t index) const noexcept {
        assert(("string index violation", index < length()));
        return mImpl[index];
    }

    [[nodiscard]]
    std::optional<float> toFloat() const noexcept;

    [[nodiscard]]
    std::optional<double> toDouble() const noexcept;

    [[nodiscard]]
    std::optional<int> toInt() const noexcept;

    [[nodiscard]]
    std::optional<int> toIntHex() const noexcept;

    [[nodiscard]]
    std::optional<unsigned> toUInt() const noexcept;

    [[nodiscard]]
    std::optional<bool> toBool() const noexcept;


    template<typename T, typename std::enable_if_t<std::is_convertible_v<T, AStringView>, bool> = true>
    [[nodiscard]]
    bool operator==(const T& t) const noexcept {
        return mImpl == AStringView(t).std();
    }

    template<typename T, typename std::enable_if_t<std::is_convertible_v<T, AStringView>, bool> = true>
    [[nodiscard]]
    bool operator!=(const T& t) const noexcept {
        return mImpl != AStringView(t).std();
    }

    [[nodiscard]]
    bool contains(char c) const noexcept {
        return find(c) != npos;
    }

    [[nodiscard]]
    bool contains(AStringView other) const noexcept {
        return find(other) != npos;
    }

};

/**
 * Exclusive-owning representation of the string which provides full string functionality set.
 */
class API_AUI_CORE AString {
private:
    friend struct std::hash<AString>;
    std::string mImpl;

    [[nodiscard]]
    AStringView makeView() const {
        return {data(), length()};
    }

public:

    using iterator = std::string::iterator;
    using const_iterator = std::string::const_iterator;
    using reverse_iterator = std::string::reverse_iterator;
    using const_reverse_iterator = std::string::const_reverse_iterator;
    constexpr static inline auto npos = std::string::npos;

    AString() = default;

    [[nodiscard]]
    const char* data() const {
        return mImpl.data();
    }

    [[nodiscard]]
    char* data() {
        return mImpl.data();
    }

    /*
    operator AStringView() const {
        return makeView();
    }*/

    AString(AStringView stringView) : mImpl(stringView.data(), stringView.length()) {}
    explicit AString(std::string&& other) : mImpl(std::forward<std::string>(other)) {}
    explicit AString(const std::string& other) : mImpl(other) {}
    explicit AString(char c, std::size_t count = 1) : mImpl(count, c) {}

    AString(const AString& other) : mImpl(other.mImpl) {}
    AString(const char* string) : mImpl(string) {}
    AString(const wchar_t* string);
    AString(const char* string, std::size_t length) : mImpl(string, length) {}
    AString(AString&& other): mImpl(std::move(other.mImpl)) {}

    template<typename Iterator>
    AString(Iterator begin, Iterator end): mImpl(begin, end) {}

    ~AString() = default;

    AString& uppercase();
    AString& lowercase();

    AString& operator=(const AString&) = default;
    AString& operator=(AString&&) = default;

    [[nodiscard]]
    AString uppercased() const {
        AString copy = *this;
        return copy.uppercase();
    }

    [[nodiscard]]
    AString lowercased() const {
        AString copy = *this;
        return copy.lowercase();
    }

    [[nodiscard]]
    bool startsWith(AStringView other) const noexcept {
        return makeView().startsWith(other);
    }

    [[nodiscard]]
    bool startsWith(char c) const noexcept {
        return makeView().startsWith(c);
    }

    [[nodiscard]]
    bool endsWith(AStringView other) const noexcept {
        return makeView().endsWith(other);
    }

    [[nodiscard]]
    bool endsWith(char c) const noexcept {
        return makeView().endsWith(c);
    }

    /**
     * Sets convents size. If internal buffer's capacity is not enough, it will be reallocated.
     * @param newCapacity new size
     */
    void resize(std::size_t newCapacity) {
        mImpl.resize(newCapacity);
    }


    /**
     * Sets capacity of the internal buffer.
     * @param newCapacity new internal buffer capacity.
     */
    void reserve(std::size_t newCapacity) {
        mImpl.reserve(newCapacity);
    }

    /**
     * @return length of the stored string.
     * @note use <a href="empty">empty</a> instead of <code>length() == 0</code>.
     */
    [[nodiscard]]
    std::size_t length() const noexcept {
        return mImpl.length();
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return mImpl.empty();
    }

    [[nodiscard]]
    auto begin() noexcept {
        return mImpl.begin();
    }

    [[nodiscard]]
    auto begin() const noexcept {
        return mImpl.begin();
    }

    [[nodiscard]]
    auto end() noexcept {
        return mImpl.end();
    }

    [[nodiscard]]
    auto end() const noexcept {
        return mImpl.end();
    }
    [[nodiscard]]
    auto rbegin() noexcept {
        return mImpl.rbegin();
    }

    [[nodiscard]]
    auto rbegin() const noexcept {
        return mImpl.rbegin();
    }

    [[nodiscard]]
    auto rend() noexcept {
        return mImpl.rend();
    }

    [[nodiscard]]
    auto rend() const noexcept {
        return mImpl.rend();
    }

    [[nodiscard]]
    AStringVector split(char c = ' ') const;

    [[nodiscard]]
    AString restrictLength(size_t s, AStringView stringAtEnd = "...") const {
        return makeView().restrictLength(s, stringAtEnd);
    }

    [[nodiscard]] AString replacedAll(const AString& from, const AString& to) const noexcept;

    [[nodiscard]] AString replacedAll(char from, char to) const {
        AString copy;
        copy.reserve(length() + 10);
        for (auto c: *this) {
            if (c == from) {
                copy << to;
            } else {
                copy << c;
            }
        }
        return copy;
    }

    AString& replaceAll(char from, char to) {
        for (auto& r : *this) {
            if (r == from) {
                r = to;
            }
        }

        return *this;
    }

    [[nodiscard]] AString replacedAll(const ASet<char>& from, char to) const noexcept {
        AString copy;
        copy.reserve(length() + 10);
        for (auto c: *this) {
            if (from.contains(c)) {
                copy << to;
            } else {
                copy << c;
            }
        }
        return copy;
    }

    [[nodiscard]]
    std::optional<float> toFloat() const noexcept {
        return makeView().toFloat();
    }

    [[nodiscard]]
    std::optional<double> toDouble() const noexcept {
        return makeView().toDouble();
    }

    [[nodiscard]]
    std::optional<int> toInt() const noexcept {
        return makeView().toInt();
    }

    [[nodiscard]]
    std::optional<int> toIntHex() const noexcept {
        return makeView().toIntHex();
    }

    [[nodiscard]]
    std::optional<unsigned> toUInt() const noexcept {
        return makeView().toUInt();
    }

    [[nodiscard]]
    std::optional<bool> toBool() const noexcept {
        return makeView().toBool();
    }

    [[nodiscard]]
    bool contains(char c) const noexcept {
        return find(c) != npos;
    }

    [[nodiscard]]
    bool contains(AStringView other) const noexcept {
        return find(other) != npos;
    }

    [[nodiscard]]
    std::size_t find(char c, std::size_t offset = 0) const noexcept {
        return mImpl.find(c, offset);
    }

    [[nodiscard]]
    std::size_t find(AStringView v, std::size_t offset = 0) const noexcept {
        return makeView().find(v, offset);
    }

    [[nodiscard]]
    std::size_t rfind(char c, std::size_t offset = npos) const noexcept {
        return mImpl.rfind(c, offset);
    }

    [[nodiscard]]
    std::size_t rfind(AStringView v, std::size_t offset = npos) const noexcept {
        return makeView().rfind(v, offset);
    }

    [[nodiscard]]
    static AString fromLatin1(const AByteBuffer& buffer);

    [[nodiscard]]
    static AString fromBuffer(const AByteBuffer& buffer);

    [[nodiscard]]
    static AString fromLatin1(const char* buffer);

    [[nodiscard]]
    static AString numberHex(int i) noexcept;

    template<typename T, std::enable_if_t<
            std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int> = 0>
    static AString number(T i) noexcept {
        if constexpr (std::is_same_v<bool, std::decay_t<T>>) {
            if (i)
                return "true";
            return "false";
        } else {
            return AString(std::to_string(i));
        }
    }

    int toNumberDec() const noexcept;

    /**
     * @return converts utf8 string to utf16.
     */
    [[nodiscard]]
    std::wstring toUtf16() const {
        return makeView().toUtf16();
    }

    /**
     * TODO STUB
     */
    const std::string& toStdString() const noexcept {
        return mImpl;
    }

    const std::string& std() const noexcept {
        return mImpl;
    }

    void removeAt(unsigned index) noexcept {
        mImpl.erase(begin() + index);
    }

    /**
     * @return copy of the string but multiple spaces in a row replaced by the single space.
     */
    AString excessSpacesRemoved() const noexcept;

    void push_back(char c) {
        mImpl.push_back(c);
    }

    AString& operator<<(char c) {
        push_back(c);
        return *this;
    }

    int compare(AStringView other) const noexcept {
        return makeView().compare(other);
    }

    bool operator<(const AString& other) const noexcept {
        return compare(other) < 0;
    }

    char& front() noexcept {
        return mImpl.front();
    }

    char& back() noexcept {
        return mImpl.back();
    }

    char front() const noexcept {
        return mImpl.front();
    }

    char back() const noexcept {
        return mImpl.back();
    }

    char& first() noexcept {
        return mImpl.front();
    }

    char& last() noexcept {
        return mImpl.back();
    }

    char first() const noexcept {
        return mImpl.front();
    }

    char last() const noexcept {
        return mImpl.back();
    }

    AString& append(AStringView s) {
        mImpl.append(s.std());
        return *this;
    }

    AString& operator+=(char c)  {
        mImpl += c;
        return *this;
    }
    AString& operator+=(AStringView c)  {
        mImpl += c.std();
        return *this;
    }

    template<typename... Args>
    AString format(Args&&... args) const {
        return makeView().format(std::forward<Args>(args)...);
    }

    AString processEscapes() const {
        return makeView().processEscapes();
    }

    [[nodiscard]]
    AStringView substr(std::size_t beginning, std::size_t count = npos) const noexcept {
        return makeView().substr(beginning, count);
    }

    [[nodiscard]]
    char operator[](std::size_t index) const noexcept {
        assert(("string index violation", index < length()));
        return mImpl[index];
    }

    template<typename T, typename std::enable_if_t<std::is_convertible_v<T, AStringView>, bool> = true>
    [[nodiscard]]
    bool operator==(const T& t) const noexcept {
        return mImpl == AStringView(t).std();
    }

    template<typename T, typename std::enable_if_t<std::is_convertible_v<T, AStringView>, bool> = true>
    [[nodiscard]]
    bool operator!=(const T& t) const noexcept {
        return mImpl != AStringView(t).std();
    }

    [[nodiscard]]
    const char* c_str() const noexcept {
        return mImpl.c_str();
    }

    /**
     * Clears the string.
     */
    void clear() noexcept {
        mImpl.clear();
    }


    [[nodiscard]]
    AStringView trimLeft(char symbol = ' ') const noexcept {
        return makeView().trimLeft(symbol);
    }

    [[nodiscard]]
    AStringView trimRight(char symbol = ' ') const noexcept {
        return makeView().trimRight(symbol);
    }

    [[nodiscard]]
    AStringView trim(char symbol = ' ') const noexcept {
        return makeView().trim(symbol);
    }

    iterator insert(const const_iterator& where, char c) {
        return mImpl.insert(where, c);
    }
    iterator insert(const const_iterator& where, AStringView s) {
        return mImpl.insert(where, s.data(), s.data() + s.length());
    }
    AString& insert(std::size_t where, char c) {
        mImpl.insert(where, 1, c);
        return *this;
    }
    AString& insert(std::size_t where, AStringView s) {
        mImpl.insert(where, s.std());
        return *this;
    }

    template<typename Iterator>
    iterator insert(const const_iterator& where, Iterator begin, Iterator end) {
        return mImpl.insert(where, begin, end);
    }

    iterator erase(const const_iterator& begin, const const_iterator& end) noexcept {
        return mImpl.erase(begin, end);
    }

    AString& erase(std::size_t where) noexcept {
        mImpl.erase(where);
        return *this;
    }

    AString& erase(std::size_t where, std::size_t count) noexcept {
        mImpl.erase(where, count);
        return *this;
    }
};

inline AStringView operator "" _as(const char* str, size_t len) {
    return {str, len};
}

inline AString AStringView::uppercased() const {
    auto copy = AString(*this);
    copy.uppercase();
    return copy;
}

inline AString AStringView::lowercased() const {
    auto copy = AString(*this);
    copy.lowercase();
    return copy;
}

inline AString AStringView::replacedAll(char from, char to) const noexcept {
    auto copy = AString(*this);
    copy.replaceAll(from, to);
    return copy;
}

inline std::ostream& operator<<(std::ostream& o, const AString& s) {
    o << s.toStdString();
    return o;
}

namespace std {
    template<>
    struct hash<AString> {
        size_t operator()(const AString& t) const {
            return hash<std::string>()(t.std());
        }
    };
}

//template<typename T1, typename T2, typename std::enable_if_t<std::is_integral_v<T>, bool> = 0>

[[nodiscard]]
inline AString operator+(const AStringView& v1, const AStringView& v2) {
    AString str;
    str.reserve(v1.length() + v2.length());
    str += v1;
    str += v2;
    return str;
}

[[nodiscard]]
inline AString operator+(const AStringView& v1, char v2) {
    AString str;
    str.reserve(v1.length() + 1);
    str += v1;
    str += v2;
    return str;
}

inline AString AStringView::str() const noexcept {
    return AString(*this);
}

inline AStringView::AStringView(const AString& str): mImpl(str.data(), str.length()) {

}

// gtest printer for AString
inline void PrintTo(const AString& s, std::ostream* stream) {
    *stream << s;
}

// gtest printer for AStringView
inline void PrintTo(AStringView s, std::ostream* stream) {
    *stream << s;
}
