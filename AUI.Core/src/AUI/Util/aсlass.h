#pragma once
#include "AUI/Common/AString.h"



template<class T>
class aclass
{
public:
	static AString name()
	{
#ifdef _WIN32
		AString s = __FUNCSIG__;
		auto openTag = s.find('<') + 7;
		auto closeTag = s.find('>');
		auto name = s.mid(openTag, closeTag - openTag);
		if (name.endsWith(" &"))
			name = name.mid(0, name.length() - 2);
		return name;
#else
		AString s = __PRETTY_FUNCTION__;
		auto b = s.find("with T = ") + 9;
		auto e = s.find("&", b);
        auto result = s.mid(b, e - b);
        return result;
#endif
	}
};
