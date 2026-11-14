/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <AUI/Render/SimpleTexturePacker.h>

namespace {
AImage white(glm::uvec2 size) {
    AImage image(size, APixelFormat::R | APixelFormat::BYTE);
    image.fill(AColor::WHITE);
    return image;
}
}   // namespace

// The first image used to be put at (0, 0) without a size check: an image larger than the initial texture (i.e., a
// large glyph on a high dpi screen) was cut, and its bottom showed whatever was in the texture memory.
TEST(TexturePacker, FirstImageLargerThanInitialTexture) {
    Util::SimpleTexturePacker packer;
    auto glyph = white({ 40, 110 });
    const auto uv = packer.insert(glyph);

    EXPECT_EQ(uv, glm::vec4(0, 0, 40, 110));
    const auto& atlas = *packer.getImage();
    ASSERT_GE(atlas.height(), 110u);
    for (unsigned y = 0; y < 110; ++y) {
        ASSERT_EQ(atlas.get({ 20, y }).r, 1.f) << "row " << y;
    }
}

TEST(TexturePacker, ImagesDoNotOverlap) {
    Util::SimpleTexturePacker packer;
    AVector<glm::vec4> uvs;
    for (int i = 0; i < 50; ++i) {
        auto image = white({ 10u + i % 7 * 9, 15u + i % 5 * 20 });
        uvs << packer.insert(image);
    }
    for (size_t i = 0; i < uvs.size(); ++i) {
        for (size_t j = i + 1; j < uvs.size(); ++j) {
            const auto& a = uvs[i];
            const auto& b = uvs[j];
            const bool overlap = a.x < b.z && b.x < a.z && a.y < b.w && b.y < a.w;
            ASSERT_FALSE(overlap) << i << " and " << j;
        }
    }
    const auto side = float(packer.getImage()->width());
    for (const auto& uv : uvs) {
        ASSERT_LE(uv.z, side);
        ASSERT_LE(uv.w, side);
    }
}
