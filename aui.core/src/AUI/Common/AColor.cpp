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
				r = static_cast<float>(s.mid(1, 1).toNumberHex()) / 15.f;
				g = static_cast<float>(s.mid(2, 1).toNumberHex()) / 15.f;
				b = static_cast<float>(s.mid(3, 1).toNumberHex()) / 15.f;
				a = 1.f;
				break;

			// #ffff
			case 5:
				r = static_cast<float>(s.mid(1, 1).toNumberHex()) / 15.f;
				g = static_cast<float>(s.mid(2, 1).toNumberHex()) / 15.f;
				b = static_cast<float>(s.mid(3, 1).toNumberHex()) / 15.f;
				a = static_cast<float>(s.mid(4, 1).toNumberHex()) / 15.f;
				break;
				
			// #ffffff
			case 7:
				r = static_cast<float>(s.mid(1, 2).toNumberHex()) / 255.f;
				g = static_cast<float>(s.mid(3, 2).toNumberHex()) / 255.f;
				b = static_cast<float>(s.mid(5, 2).toNumberHex()) / 255.f;
				a = 1.f;
				break;
				
			// #ffffffff
			case 9:
				r = static_cast<float>(s.mid(1, 2).toNumberHex()) / 255.f;
				g = static_cast<float>(s.mid(3, 2).toNumberHex()) / 255.f;
				b = static_cast<float>(s.mid(5, 2).toNumberHex()) / 255.f;
				a = static_cast<float>(s.mid(7, 2).toNumberHex()) / 255.f;
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
