// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include "AUI/Common/AString.h"



template<class T>
class AClass
{
public:
	static AString name()
	{
#if defined(_MSC_VER)
		AString s = __FUNCSIG__;
		auto openTag = s.find('<') + 1;
		auto closeTag = s.find('>');
		auto name = s.substr(openTag, closeTag - openTag);
        name = name.substr(name.rfind(' ') + 1);
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
        if (p != AString::NPOS) {
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