#pragma once
#include <AUI/Common/AString.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/IO/APath.h>
#include <glm/glm.hpp>
#include <cstring>

namespace aui {

    template<typename T>
    inline AString to_string(const T& t) {
        return std::to_wstring(t);
    };

    template<>
    inline AString to_string(const char* const& t) {
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
            inline static constexpr size_t format_length(T t) {
                return glm::pow(2llu, sizeof(T)) + 1;
            }
        };
        template<>
        struct type_length<wchar_t*> {
            inline static size_t format_length(wchar_t* t) {
                return wcslen(t);
            }
        };
        template<>
        struct type_length<char*> {
            inline static size_t format_length(char* t) {
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
        struct type_length<APath> {
            inline static size_t format_length(const APath& t) {
                return t.length();
            }
        };


        template <typename Arg>
        inline size_t format_length(Arg&& arg) {
            return type_length<std::remove_const_t<std::remove_reference_t<Arg>>>::format_length(std::forward<Arg>(arg));
        }
        template <typename Arg, typename... Args>
        inline size_t format_length(Arg&& arg, Args&&... args) {
            return type_length<std::remove_const_t<std::remove_reference_t<Arg>>>::format_length(std::forward<Arg>(arg))
                + format_length(std::forward<Args>(args)...);
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
     * \brief Форматирование строки.
     *        Форматирование строки аналогично форматированию в Rust
     * @return отформатированная строка
     */
    template <typename... Args>
    inline AString format(const AString& format, Args&&... args) {
        AStringVector stringArgs;
        stringArgs.reserve(sizeof...(Args));
        detail::format::to_string_array(stringArgs, std::forward<Args>(args)...);

        AString result;
        size_t bufferLength = format.length() + detail::format::format_length(std::forward<Args>(args)...) + 100;
        result.reserve(bufferLength);

        bool ignoreCurledBrackets = false;

        size_t parameterIndex = 0;

        for (auto i = format.begin(); i != format.end(); ++i) {
            wchar_t c = *i;
            switch (c) {
                case '{':
                    if (ignoreCurledBrackets) {
                        ignoreCurledBrackets = false;
                    } else {
                        // пошёл формат строки.
                        c = *(++i);
                        assert(c == '}');

                        result += stringArgs[parameterIndex++];
                        break;
                    }
                default:
                    result += c;
                    break;
                case '\\':
                    ignoreCurledBrackets = true;
                    break;
            }
        }

        return result;
    }
}

template<typename... Args>
inline AString AString::format(Args&& ... args) {
    return aui::format(*this, std::forward<Args>(args)...);
}
