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

#include <AUI/Enum/ATextAlign.h>
#include <AUI/Font/AFont.h>
#include <AUI/Font/FontRendering.hpp>
#include <AUI/Platform/AFontManager.h>
#include <AUI/Common/AColor.h>

/**
 * @brief Font style.
 * @ingroup views
 */
struct API_AUI_VIEWS AFontStyle {
    _<AFont> font = AFontManager::inst().getDefaultFont();
    unsigned size = 12;
    FontRendering fontRendering = FontRendering::SUBPIXEL;
    AColor color = AColor::BLACK;
    bool lineThrough = false;
    bool overline = false;
    bool underline = false;
    bool italic = false;
    bool bold = false;
    ATextAlign align = ATextAlign::LEFT;
    int lineSpacing = 0;

    AFont::FontEntry getFontEntry() const { return font->getFontEntry({ .size = size, .fr = fontRendering }); }

    template <class Iterator>
    int getWidth(Iterator begin, Iterator end) const {
        return font->length(getFontEntry(), std::move(begin), std::move(end));
    }

    template <class Container>
    int getWidth(const Container& container) const {
        return getWidth(container.begin(), container.end());
    }

    int getWidth(AStringView text) const;
    int getWidth(std::u32string_view text) const;

    int getLineHeight() const { return size + lineSpacing; }

    int getSpaceWidth() const { return font->getSpaceWidth(size); }

    AFont::Character& getCharacter(AChar c) const { return font->getCharacter(getFontEntry(), c); }

    int getAscenderHeight() const { return font->getAscenderHeight(size); }

    int getDescenderHeight() const { return font->getDescenderHeight(size); }

    void setFont(const _<AFont>& fontIn) { font = fontIn; }

    void setSize(unsigned int sizeIn) { size = sizeIn; }

    void setFontRendering(FontRendering fontRenderingIn) { fontRendering = fontRenderingIn; }

    AFontStyle withSize(unsigned sizeIn) const {
        return {
            .font = font,
            .size = sizeIn,
            .fontRendering = fontRendering,
            .color = color,
            .lineThrough = lineThrough,
            .overline = overline,
            .underline = underline,
            .italic = italic,
            .bold = bold,
            .align = align,
            .lineSpacing = lineSpacing
        };
    }

    operator AFont::FontEntry() const { return getFontEntry(); }

    bool operator==(const AFontStyle&) const noexcept = default;
    bool operator!=(const AFontStyle&) const noexcept = default;
};
