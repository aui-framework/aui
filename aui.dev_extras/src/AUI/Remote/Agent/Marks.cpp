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

#include "Marks.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace aui::remote::marks {

namespace {

constexpr int GLYPH_WIDTH = 5;
constexpr int GLYPH_HEIGHT = 7;

// rows from top to bottom, the most significant of 5 bits is the leftmost column
constexpr std::array<std::array<std::uint8_t, GLYPH_HEIGHT>, 10> DIGITS = { {
  { 0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110 },   // 0
  { 0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110 },   // 1
  { 0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111 },   // 2
  { 0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110 },   // 3
  { 0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010 },   // 4
  { 0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110 },   // 5
  { 0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110 },   // 6
  { 0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000 },   // 7
  { 0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110 },   // 8
  { 0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100 },   // 9
} };

// saturated and dark enough for white digits
const std::array<AColor, 8> PALETTE = {
    0xe6194b_rgb, 0x4363d8_rgb, 0x2e8b2e_rgb, 0xd2600a_rgb,
    0x911eb4_rgb, 0x00808a_rgb, 0xc0179e_rgb, 0x8a5a14_rgb,
};

const AColor OUTLINE = 0x000000_rgb;
const AColor WHITE = 0xffffff_rgb;

class Canvas {
public:
    explicit Canvas(AImage& image) : mImage(image), mSize(image.size()) {}

    void fill(ARect<int> rect, const AColor& color) {
        const auto from = glm::max(rect.p1, glm::ivec2(0));
        const auto to = glm::min(rect.p2, mSize);
        for (int y = from.y; y < to.y; ++y) {
            for (int x = from.x; x < to.x; ++x) {
                mImage.set({ x, y }, color);
            }
        }
    }

    void frame(ARect<int> rect, int width, const AColor& color) {
        fill({ rect.p1, { rect.p2.x, rect.p1.y + width } }, color);
        fill({ { rect.p1.x, rect.p2.y - width }, rect.p2 }, color);
        fill({ rect.p1, { rect.p1.x + width, rect.p2.y } }, color);
        fill({ { rect.p2.x - width, rect.p1.y }, rect.p2 }, color);
    }

    [[nodiscard]] glm::ivec2 size() const noexcept { return mSize; }

private:
    AImage& mImage;
    glm::ivec2 mSize;
};

bool intersects(const ARect<int>& a, const ARect<int>& b) {
    return a.p1.x < b.p2.x && b.p1.x < a.p2.x && a.p1.y < b.p2.y && b.p1.y < a.p2.y;
}

ARect<int> moved(ARect<int> rect, glm::ivec2 to) { return ARect<int>::fromTopLeftPositionAndSize(to, rect.size()); }

ARect<int> clampedInto(ARect<int> rect, glm::ivec2 bounds) {
    const auto size = rect.size();
    auto position = glm::clamp(rect.p1, glm::ivec2(0), glm::max(bounds - size, glm::ivec2(0)));
    return ARect<int>::fromTopLeftPositionAndSize(position, size);
}

}   // namespace

void draw(AImage& image, const AVector<Mark>& marks, float pixelsPerDp) {
    Canvas canvas(image);
    const int line = std::max(2, int(std::lround(1.2f * pixelsPerDp)));
    const int unit = std::max(2, int(std::lround(1.2f * pixelsPerDp)));   // a pixel of the digit font
    const int padding = unit;

    for (const auto& mark : marks) {
        const auto& color = PALETTE[mark.label % PALETTE.size()];
        canvas.frame(mark.rect, line, color);
        // a thin light line inside the box keeps it visible on dark backgrounds of the same hue
        canvas.frame({ mark.rect.p1 + glm::ivec2(line), mark.rect.p2 - glm::ivec2(line) }, 1, WHITE);
    }

    AVector<ARect<int>> placed;
    for (const auto& mark : marks) {
        const auto text = AString::number(mark.label).toStdString();
        const auto glyphs = int(text.size());
        const glm::ivec2 plateSize { glyphs * GLYPH_WIDTH * unit + (glyphs - 1) * unit + 2 * padding + 2,
                                     GLYPH_HEIGHT * unit + 2 * padding + 2 };
        const auto& box = mark.rect;
        const auto plate = ARect<int>::fromTopLeftPositionAndSize({}, plateSize);
        // outside above, inside, below, inside at the right, outside at the left: the first spot that is free
        const std::array candidates = {
            moved(plate, { box.p1.x, box.p1.y - plateSize.y }),
            moved(plate, box.p1),
            moved(plate, { box.p1.x, box.p2.y }),
            moved(plate, { box.p2.x - plateSize.x, box.p1.y }),
            moved(plate, { box.p1.x - plateSize.x, box.p1.y }),
        };
        const auto coversOtherBox = [&](const ARect<int>& candidate) {
            return std::any_of(marks.begin(), marks.end(), [&](const Mark& other) {
                return &other != &mark && intersects(other.rect, candidate);
            });
        };
        const auto coversPlate = [&](const ARect<int>& candidate) {
            return std::any_of(placed.begin(), placed.end(), [&](const auto& p) { return intersects(p, candidate); });
        };
        auto chosen = clampedInto(candidates[1], canvas.size());
        // a spot that covers neither another plate nor another box; failing that, at least not another plate
        [&] {
            for (bool strict : { true, false }) {
                for (const auto& candidate : candidates) {
                    if (clampedInto(candidate, canvas.size()).p1 != candidate.p1) {
                        continue;   // does not fit into the image there
                    }
                    if (!coversPlate(candidate) && !(strict && coversOtherBox(candidate))) {
                        chosen = candidate;
                        return;
                    }
                }
            }
        }();
        placed << chosen;

        canvas.fill(chosen, OUTLINE);
        canvas.fill({ chosen.p1 + glm::ivec2(1), chosen.p2 - glm::ivec2(1) }, PALETTE[mark.label % PALETTE.size()]);
        glm::ivec2 pen = chosen.p1 + glm::ivec2(1 + padding);
        for (char c : text) {
            const auto& rows = DIGITS[c - '0'];
            for (int y = 0; y < GLYPH_HEIGHT; ++y) {
                for (int x = 0; x < GLYPH_WIDTH; ++x) {
                    if (rows[y] & (1 << (GLYPH_WIDTH - 1 - x))) {
                        const auto p = pen + glm::ivec2(x, y) * unit;
                        canvas.fill({ p, p + glm::ivec2(unit) }, WHITE);
                    }
                }
            }
            pen.x += (GLYPH_WIDTH + 1) * unit;
        }
    }
}

}   // namespace aui::remote::marks
