// AUI Framework - Declarative UI toolkit for modern C++17
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

#pragma once
#include "AUI/Common/AString.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include <AUI/Common/AMap.h>

#include <utility>

/**
 * @brief Uniform Resource Locator implementation.
 * @ingroup core
 * @details
 * Handles standard web url (schema://host:port/path).
 */
class API_AUI_CORE AUrl
{
private:
	AString mSchema;
	AString mPath;

    static AMap<AString, std::function<_<IInputStream>(const AUrl&)>>& resolvers();

public:
	AUrl(AString full);
	inline AUrl(const char* full): AUrl(AString(full)) {}

    AUrl(AString schema, AString path) : mSchema(std::move(schema)), mPath(std::move(path)) {}

    static AUrl file(const AString& file) {
        return { "file", file };
    }

    [[nodiscard]]
	_<IInputStream> open() const;

    [[nodiscard]]
	const AString& path() const noexcept
	{
		return mPath;
	}

    [[nodiscard]]
	const AString& schema() const noexcept
	{
		return mSchema;
	}

    [[nodiscard]]
	AString full() const {
	    return mSchema + "://" + mPath;
	}

    bool operator<(const AUrl& u) const {
        return full() < u.full();
    }

	static void registerResolver(const AString& protocol, const std::function<_<IInputStream>(const AUrl&)>& factory);
};



inline AUrl operator"" _url(const char* input, size_t s)
{
    return AUrl(std::string{input, input + s});
}