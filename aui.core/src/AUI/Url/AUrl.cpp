﻿// AUI Framework - Declarative UI toolkit for modern C++20
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

#include "AUrl.h"

#include <functional>
#include <AUI/IO/AFileInputStream.h>

#include "AUI/Common/AMap.h"
#include "AUI/Util/ABuiltinFiles.h"

AUrl::AUrl(AString full)
{
	auto posColon = full.find(':');
	if (posColon == AString::NPOS)
	{
		mPath = std::move(full);
		if (mPath.startsWith("./")) {
			mPath = mPath.substr(2);
		}
        mSchema = "file";
	} else
	{
        mSchema = full.substr(0, posColon);
		if (mSchema.empty())
		{
            mSchema = "builtin";
			mPath = full.substr(posColon + 1);
		} else
		{
            mPath = full.substr(posColon + 3);
		}
	}
}

AMap<AString, std::function<_<IInputStream>(const AUrl&)>>& AUrl::resolvers() {
    static AMap<AString, std::function<_<IInputStream>(const AUrl&)>> storage = {
        {"builtin", [](const AUrl& u) {
            return ABuiltinFiles::open(u.path());
        }},
        {"file",    [](const AUrl& u) {
            return _new<AFileInputStream>(u.path());
        }},
    };
    return storage;
}
_<IInputStream> AUrl::open() const {
	if (auto c = resolvers().contains(mSchema)) {
		if (auto is = c->second(*this))
			return is;
	}
	throw AIOException("could not open url: " + full());
}


void AUrl::registerResolver(const AString& protocol, const std::function<_<IInputStream>(const AUrl&)>& factory) {
    resolvers()[protocol] = factory;
}
