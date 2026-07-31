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

#include <ft2build.h>
#include <freetype/freetype.h>
#include <mutex>

class FreeType {
public:
    /**
     * @brief Mutex serializing every FreeType call on the shared FT_Library:
     *        library-level operations (FT_New_Face, FT_New_Memory_Face,
     *        FT_Done_Face) and all FT_Face operations on both the primary
     *        shared face (mFace) and the fallback faces. FreeType library
     *        state (e.g. the shared raster pool) is not thread-safe, so calls
     *        on faces of the same library must not run concurrently.
     *        Fallback faces additionally use a per-face mutex so that
     *        FT_Done_Face cannot overlap an in-flight glyph load on the same
     *        face.
     */
    static std::mutex sFaceMutex;

private:
	FT_Library ft;
public:
	FreeType();
	~FreeType();
	FreeType(const FreeType&) = delete;
	FT_Library getFt() const;
};