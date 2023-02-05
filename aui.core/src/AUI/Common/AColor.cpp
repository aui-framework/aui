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

#include "AColor.h"


#include "AString.h"
#include "AMap.h"

AColor::AColor(const AString& s)
{
	if (!s.empty())
	{
		if (s.startsWith("#"))
		{
			switch (s.length())
			{
			// #, etc...
			default:
				break;

			// #fff
			case 4:
				r = static_cast<float>(s.substr(1, 1).toNumberHex()) / 15.f;
				g = static_cast<float>(s.substr(2, 1).toNumberHex()) / 15.f;
				b = static_cast<float>(s.substr(3, 1).toNumberHex()) / 15.f;
				a = 1.f;
				break;

			// #ffff
			case 5:
				r = static_cast<float>(s.substr(1, 1).toNumberHex()) / 15.f;
				g = static_cast<float>(s.substr(2, 1).toNumberHex()) / 15.f;
				b = static_cast<float>(s.substr(3, 1).toNumberHex()) / 15.f;
				a = static_cast<float>(s.substr(4, 1).toNumberHex()) / 15.f;
				break;
				
			// #ffffff
			case 7:
				r = static_cast<float>(s.substr(1, 2).toNumberHex()) / 255.f;
				g = static_cast<float>(s.substr(3, 2).toNumberHex()) / 255.f;
				b = static_cast<float>(s.substr(5, 2).toNumberHex()) / 255.f;
				a = 1.f;
				break;
				
			// #ffffffff
			case 9:
				r = static_cast<float>(s.substr(1, 2).toNumberHex()) / 255.f;
				g = static_cast<float>(s.substr(3, 2).toNumberHex()) / 255.f;
				b = static_cast<float>(s.substr(5, 2).toNumberHex()) / 255.f;
				a = static_cast<float>(s.substr(7, 2).toNumberHex()) / 255.f;
				break;
			}
		}
		else
		{
			AMap<AString, AColor> colors = {
				{"red", 0xff0000ffu},
				{"green", 0x00ff00ffu},
				{"blue", 0x0000ffffu},
			};
			auto it = colors.find(s);
			if (it != colors.end())
			{
				r = it->second.r;
				g = it->second.g;
				b = it->second.b;
				a = it->second.a;
			}
		}
	}
}

AString AColor::toString() const
{
	char buf[16];
	sprintf(buf, "#%02x%02x%02x%02x", static_cast<unsigned char>(r * 255.f),
										    static_cast<unsigned char>(g * 255.f),
										    static_cast<unsigned char>(b * 255.f),
										    static_cast<unsigned char>(a * 255.f));
	return buf;
}

float AColor::readabilityOfForegroundColor(const AColor& foreground) {
    auto delta = glm::abs(glm::vec3(foreground) - glm::vec3(*this));
    return delta.x + delta.y + delta.z;
}
