/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

	[[nodiscard]] AArc<AFontFamily> getDefaultFamily() {
        return mDefaultFamily;
    }
	[[nodiscard]] AArc<AFont> getDefaultFont() {
        return mDefaultFont;
    }

    [[nodiscard]]
    AArc<AFontFamily> getFontFamily(const AString& name) const {
        if (auto c = mFamilies.contains(name)) {
            return c->second;
        }
        return nullptr;
    }
    [[nodiscard]]
    AArc<AFont> getFont(const AUrl& url) {
        if (auto c = mLoadedFont.contains(url)) {
            return c->second;
        }
        return mLoadedFont[url] = loadFont(url);
    }
private:
    AMap<AUrl, AArc<AFont>> mLoadedFont;
    AMap<AString, AArc<AFontFamily>> mFamilies;
    AArc<FreeType> mFreeType;
    AArc<AFontFamily> mDefaultFamily;
    AArc<AFont> mDefaultFont;

	AString getPathToFont(const AString& family);

    AArc<AFont> loadFont(const AUrl& url);


	friend class AFont;
};
