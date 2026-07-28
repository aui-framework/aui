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

#include "AUI/Enum/ATextAlign.h"
#include "AFont.h"
#include "AUI/Platform/AFontManager.h"
#include "AUI/Render/FontRendering.h"
#include "AUI/Common/AColor.h"


class AString;


struct API_AUI_VIEWS AFontStyle {
    mutable _<AFont> font = AFontManager::inst().getDefaultFont();
    unsigned size = 12;
    bool formatting = false;
    ATextAlign align = ATextAlign::LEFT;
    bool bold = false;
    bool italic = false;

    FontRendering fontRendering = FontRendering::SUBPIXEL;
    float lineSpacing = 0.5f;


    size_t getWidth(AStringView text) const;

    size_t getWidth(std::u32string_view text) const;

    template<class Iterator>
    size_t getWidth(Iterator begin, Iterator end) const {
        return font->length(*this, std::move(begin), std::move(end));
    }

    AFont::Character& getCharacter(char32_t c) {
        return font->getCharacter(getFontEntry(), c);
    }

    auto getAscenderHeight() const {
        return font->getAscenderHeight(size);
    }
    auto getDescenderHeight() const {
        return font->getDescenderHeight(size);
    }

    [[nodiscard]]
    size_t getSpaceWidth() const {
        return font->getSpaceWidth(size);
    }

    size_t getLineHeight() const;

    AFont::FontEntry getFontEntry() const {
        return font->getFontEntry({size, fontRendering});
    }

    operator AFont::FontEntry() const {
        return getFontEntry();
    }

    bool operator==(const AFontStyle&) const noexcept = default;
    bool operator!=(const AFontStyle&) const noexcept = default;
};
