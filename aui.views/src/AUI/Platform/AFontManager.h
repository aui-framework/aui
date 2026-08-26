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
#include <AUI/Common/AVector.h>
#include <AUI/Common/AByteBuffer.h>
#include <cstddef>
#include <cstdint>
#include <memory>
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
public:
    /**
     * A candidate font path with an optional face index (for TTC collections).
     */
    struct FallbackCandidate {
        AString path;
        int faceIndex = 0;
        AByteBuffer data;
    };

private:
    AMap<AUrl, _<AFont>> mLoadedFont;
    AMap<AString, _<AFontFamily>> mFamilies;
    _<FreeType> mFreeType;
    _<AFontFamily> mDefaultFamily;
    _<AFont> mDefaultFont;

    /**
     * A loaded fallback face. data keeps the font file bytes alive:
     * FT_New_Memory_Face does not copy the buffer, so it must outlive the face.
     */
    struct FallbackFace {
        FT_FaceRec_* face = nullptr;
        AByteBuffer data;
    };

    AVector<FallbackFace> mFallbackFaces;
    AVector<FallbackCandidate> mDeferredCandidates;
    bool mFallbackAttempted = false;

    /**
     * @brief Loads a single fallback face from the given candidate.
     * @return true if the face was loaded successfully.
     */
    bool loadOneFallback(FallbackCandidate candidate);

    /**
     * @brief Returns the platform's CJK fallback candidates, best first.
     *        Implemented per platform in
     *        AUI/Platform/<platform>/AFontManagerImpl.cpp. May log the
     *        reason and return an empty list when no candidates are available.
     */
    static AVector<FallbackCandidate> fallbackCandidates();

    void ensureFallbackFaceLoaded();

    /**
     * @brief Pre-warms the primary fallback face (eagerly loaded on startup).
     */
    void initFallback();

    /**
     * @brief Returns the first fallback face that contains the given codepoint,
     *        lazily loading deferred candidates if necessary.
     */
    [[nodiscard]]
    FT_FaceRec_* getFallbackFace(char32_t codepoint);

	AString getPathToFont(const AString& family);

    _<AFont> loadFont(const AUrl& url);

	friend class AFont;
};
