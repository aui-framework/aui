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
#include <AUI/Common/AString.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/IO/APath.h>
#include <glm/glm.hpp>
#include <cstring>

namespace aui {

    template<typename T>
    inline AString to_string(const T& t) {
        if constexpr (std::is_same_v<T, std::string>) {
            return t;
        } else {
            return std::to_wstring(t);
        }
    }

    template<typename T>
    inline AString to_string(T* t) {
        char buf[0xff];
        sprintf(buf, "%p", t);
        return buf;
    }
    template<>
    inline AString to_string(const char* t) {
        return t;
    }
    template<>
    inline AString to_string(char* t) {
        return t;
    }

    template<>
    inline AString to_string(const char* const& t) {
        return t;
    };
    template<>
    inline AString to_string(char* const& t) {
        return t;
    };
    template<>
    inline AString to_string(const AString& t) {
        return t;
    };

    template<>
    inline AString to_string(const APath& t) {
        return t;
    };


    namespace detail::format {
        template<typename T>
        struct type_length {
            inline static constexpr size_t format_length(const T& t) {
                return glm::ceil(glm::log(glm::pow(2llu, sizeof(T) * 8)) / glm::log(10)) + 1;
            }
        };
        template<>
        struct type_length<wchar_t*> {
            inline static size_t format_length(const wchar_t* t) {
                return wcslen(t);
            }
        };
        template<>
        struct type_length<char*> {
            inline static size_t format_length(const char* t) {
                return strlen(t);
            }
        };
        template<>
        struct type_length<AString> {
            inline static size_t format_length(const AString& t) {
                return t.length();
            }
        };
        template<>
        struct type_length<std::string> {
            inline static size_t format_length(const std::string& t) {
                return t.length();
            }
        };
        template<>
        struct type_length<APath> {
            inline static size_t format_length(const APath& t) {
                return t.length();
            }
        };


        template <typename Arg>
        inline size_t format_length(const Arg& arg) {
            return type_length<std::remove_const_t<std::remove_reference_t<Arg>>>::format_length(arg);
        }
        template <typename Arg, typename... Args>
        inline size_t format_length(const Arg& arg, const Args&... args) {
            return type_length<std::remove_const_t<std::remove_reference_t<Arg>>>::format_length(arg)
                + format_length(args...);
        }


        template <typename Arg>
        inline void to_string_array(AStringVector& dst, Arg&& arg1) {
            dst << to_string(std::forward<Arg>(arg1));
        }
        template <typename Arg, typename... Args>
        inline void to_string_array(AStringVector& dst, Arg&& arg1, Args&&... args) {
            dst << to_string(std::forward<Arg>(arg1));
            to_string_array(dst, std::forward<Args>(args)...);
        }
    }


    /**
     * @brief Formats string.
     *        String formatting is inspired by Rust's string formatting rules.
     * @return formatted string.
     */
    template <typename... Args>
    inline AString format(const AString& format, Args&&... args) {
        AStringVector stringArgs;
        stringArgs.reserve(sizeof...(Args));
        detail::format::to_string_array(stringArgs, std::forward<Args>(args)...);

        AString result;
        size_t bufferLength = format.length() + detail::format::format_length(std::forward<Args>(args)...) + 100;
        result.reserve(bufferLength);

        bool backslashEscaping = false;

        size_t parameterIndex = 0;

        for (auto i = format.begin(); i != format.end(); ++i) {
            wchar_t c = *i;
            switch (c) {
                case '{':
                    if (backslashEscaping) {
                        backslashEscaping = false;
                    } else {
                        // inside {} format but it's not supported :)
                        c = *(++i);
                        assert(c == '}');

                        result += stringArgs[parameterIndex++];
                        break;
                    }
                default:
                    if (backslashEscaping) {
                        result += '\\';
                    }
                    result += c;
                    backslashEscaping = false;
                    break;
                case '\\':
                    if (backslashEscaping) {
                        result += '\\';
                        result += '\\';
                        backslashEscaping = false;
                    } else {
                        backslashEscaping = true;
                    }
                    break;
            }
        }

        return result;
    }
}

template<typename... Args>
inline AString AString::format(Args&& ... args) const {
    return aui::format(*this, std::forward<Args>(args)...);
}


struct AStringFormatHelper {
    AString string;

    template<typename... Args>
    inline AString operator()(Args&& ... args) {
        return aui::format(string, std::forward<Args>(args)...);
    }
};

inline AStringFormatHelper operator"" _format(const char* str, size_t len)
{
    return {str};
}
