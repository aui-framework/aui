/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AUrl.h"

#include <functional>
#include <AUI/IO/AFileInputStream.h>

#include "AUI/Common/AMap.h"
#include "AUI/Util/ABuiltinFiles.h"


#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AStringStream.h"

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

AMap<AString, AVector<AUrl::Resolver>>& AUrl::resolvers() {
    static AMap<AString, AVector<Resolver>> storage = {
        {"builtin", {[](const AUrl& u) {
            return ABuiltinFiles::open(u.path());
        }}},
        {"file",    {[](const AUrl& u) {
            return _new<AFileInputStream>(u.path());
        }}},
        {"base64", {[](const AUrl& u) {
            auto decoded = AByteBuffer::fromBase64String(u.path());
            return _new<AStringStream>(std::string(decoded.data(), decoded.size()));
        }}},
    };
    return storage;
}
_<IInputStream> AUrl::open() const {
	if (const auto& resolverSet = resolvers().contains(mSchema)) {
		for (const auto& resolver : aui::reverse_iterator_wrap(resolverSet->second)) {
			if (auto is = resolver(*this))
				return is;
		}
	}
	throw AIOException("could not open url: " + full());
}


void AUrl::registerResolver(const AString& protocol, Resolver resolver) {
    resolvers()[protocol] << std::move(resolver);
}
