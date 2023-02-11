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

#pragma once

#include "AUI/Common/SharedPtr.h"
#include "AUI/Util/Manager.h"
#include "AUI/Font/AFontFamily.h"
#include "AUI/Font/AFont.h"

class API_AUI_VIEWS AFontManager {
public:
	AFontManager();
    AFontManager(const AFontManager&) = delete;
    virtual ~AFontManager();

    static AFontManager& inst();

	[[nodiscard]] _<AFontFamily> getDefaultFamily() {
        return mDefaultFamily;
    }
	[[nodiscard]] _<AFont> getDefaultFont() {
        return mDefaultFont;
    }

    [[nodiscard]]
    _<AFontFamily> getFontFamily(const AString& name) const {
        if (auto c = mFamilies.contains(name)) {
            return c->second;
        }
        return nullptr;
    }
    [[nodiscard]]
    _<AFont> getFont(const AUrl& url) {
        if (auto c = mLoadedFont.contains(url)) {
            return c->second;
        }
        return mLoadedFont[url] = loadFont(url);
    }
private:
    AMap<AUrl, _<AFont>> mLoadedFont;
    AMap<AString, _<AFontFamily>> mFamilies;
    _<FreeType> mFreeType;
    _<AFontFamily> mDefaultFamily;
    _<AFont> mDefaultFont;

	AString getPathToFont(const AString& font);

    _<AFont> loadFont(const AUrl& url);


	friend class AFont;
};
