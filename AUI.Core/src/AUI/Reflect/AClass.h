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
		auto openTag = s.find('<') + 7;
		auto closeTag = s.find('>');
		auto name = s.mid(openTag, closeTag - openTag);
		if (name.endsWith(" &"))
			name = name.mid(0, name.length() - 2);
		return name;
#elif defined(__ANDROID__)
		AString s = __PRETTY_FUNCTION__;
		auto b = s.find("=") + 1;
		auto e = s.find("&", b);
		auto result = s.mid(b, e - b);
		result = result.trim();
		return result;
#else
		AString s = __PRETTY_FUNCTION__;
		auto b = s.find("with T = ") + 9;
		auto e = s.find("&", b);
        auto result = s.mid(b, e - b);
        return result;
#endif
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