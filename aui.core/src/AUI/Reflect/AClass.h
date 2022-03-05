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
#include "AUI/Common/AString.h"
#include "AUI/Common/AStringVector.h"



template<class T>
class AClass
{
public:
	static AString name()
	{
#if defined(_MSC_VER)
		AStringView s = __FUNCSIG__;
		auto openTag = s.find('<') + 1;
		auto closeTag = s.find('>');
		auto name = s.substr(openTag, closeTag - openTag).split(' ').last();
		if (name.endsWith(" &"))
			name = name.substr(0, name.length() - 2);
		return name;
#elif AUI_PLATFORM_ANDROID
		AString s = __PRETTY_FUNCTION__;
		auto b = s.find("=") + 1;
		auto e = s.find("&", b);
		auto result = s.substr(b, e - b);
		result = result.trim();
		return result;
#else
		AString s = __PRETTY_FUNCTION__;
		auto b = s.find("with T = ") + 9;
        return { s.begin() + b, s.end() - 1 };
#endif
	}

	static AString nameWithoutNamespace() {
        auto s = name();
        auto p = s.rfind("::");
        if (p != AString::npos) {
            return {s.begin() + p + 2, s.end()};
        }
        return s;
	}

	static AString toString(const T& t) {
        return "<object of type " + name() + ">";
	}
};

template<>
inline AString AClass<AString>::toString(const AString& t) {
    return "\"" + t + "\"";
}
template<>
inline AString AClass<int>::toString(const int& t) {
    return AString::number(t);
}