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
    template<class UnicodeString, class Callback>
    void walkString(const glm::ivec2& position, const UnicodeString& text, int& outAdvanceX, int& outAdvanceY, Callback& callback) const {
        auto fe = getFontEntry();
        const bool hasKerning = font->isHasKerning();
        const float spaceWidth = float(getSpaceWidth());
        const int lineHeight = int(getLineHeight());

        // Positions are floored, not truncated: advance can be negative when
        // text is scrolled/clipped off the left edge. Matches SoftwareRenderer.
        const auto toPixel = [](float v) { return int(glm::floor(v)); };

        const auto kerningFor = [&](auto i, AChar c) -> float {
            if (!hasKerning) return 0.f;
            auto next = std::next(i);
            if (next == text.end()) return 0.f;
            return font->getKerning(c, *next, size).x;
        };

        int advanceX = position.x;
        int advanceY = position.y;
        float advance = advanceX;
        for (auto i = text.begin(); i != text.end(); ++i) {
            AChar c = *i;
            if (c == '\n') {
                callback.onSymbolAdded({toPixel(advance), advanceY});
                advanceX = (glm::max)(advanceX, int(glm::ceil(advance)));
                advance = position.x;
                advanceY += lineHeight;
                callback.onNextLine();
            } else {
                AFont::Character& ch = font->getCharacter(fe, c);
                if (ch.empty()) {
                    callback.onSymbolAdded({toPixel(advance), advanceY});
                    advance += kerningFor(i, c);
                    advance += ch.emptyAdvance(spaceWidth);
                    continue;
                }
                glm::ivec2 pos{ toPixel(advance), advanceY };
                pos.x += ch.horizontal.bearing.x;
                pos.y -= ch.horizontal.bearing.y;
                callback.onSymbolAdded({pos});
                if ((advance >= 0 && advance <= 99999) /* || gui3d */) {
                    callback.onGlyph(pos, ch, fe, c);
                }

                advance += kerningFor(i, c);

                advance += ch.horizontal.advance;
            }
        }

        callback.onSymbolAdded({toPixel(advance), advanceY});

        outAdvanceX = (glm::max)(outAdvanceX, (glm::max)(advanceX, int(glm::ceil(advance))));
        outAdvanceY = (glm::max)(outAdvanceY, advanceY + lineHeight);
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
