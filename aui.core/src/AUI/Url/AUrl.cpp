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

#include "AUrl.h"

#include <functional>
#include <AUI/IO/AFileInputStream.h>

#include "AUI/Common/AMap.h"
#include "AUI/Util/ABuiltinFiles.h"

AUrl::AUrl(const AString& full)
{
	auto posColon = full.find(':');
	if (posColon == AString::NPOS)
	{
		mPath = full;
		if (mPath.startsWith("./")) {
			mPath = mPath.mid(2);
		}
		mProtocol = "file";
	} else
	{
		mProtocol = full.mid(0, posColon);
		if (mProtocol.empty())
		{
			mProtocol = "builtin";
			mPath = full.mid(posColon + 1);
		} else
		{
            mPath = full.mid(posColon + 3);
		}
	}
}

AMap<AString, std::function<_<IInputStream>(const AUrl&)>>& AUrl::resolvers() {
    static AMap<AString, std::function<_<IInputStream>(const AUrl&)>> storage = {
        {"builtin", [](const AUrl& u) {
            return ABuiltinFiles::open(u.getPath());
        }},
        {"file",    [](const AUrl& u) {
            return _new<AFileInputStream>(u.getPath());
        }},
    };
    return storage;
}
_<IInputStream> AUrl::open() const {
	if (auto c = resolvers().contains(mProtocol)) {
		if (auto is = c->second(*this))
			return is;
	}
	throw AIOException("could not open url: " + getFull());
}


void AUrl::registerResolver(const AString& protocol, const std::function<_<IInputStream>(const AUrl&)>& factory) {
    resolvers()[protocol] = factory;
}
