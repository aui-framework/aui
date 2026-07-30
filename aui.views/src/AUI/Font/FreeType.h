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
     * @brief Mutex serializing FT_New_Face, FT_New_Memory_Face, and FT_Done_Face
     *        calls on the shared FT_Library. FreeType requires library-wide
     *        serialization for face creation/destruction when the same library
     *        is used from multiple threads.
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