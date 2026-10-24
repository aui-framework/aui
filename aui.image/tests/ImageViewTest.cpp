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

#include <gtest/gtest.h>
#include <AUI/Image/AImage.h>
#include <AUI/Image/AImageView.h>

TEST(ImageView, Stride) {
    APixelFormat pf = APixelFormat::R8G8B8A8_UNORM;
    std::vector<uint8_t> some_bytes(100 * 100 * ::bytesPerPixel(pf));
    AByteBufferView data(reinterpret_cast<const char*>(some_bytes.data()), some_bytes.size());

    AImageView view_big(data, 100 * ::bytesPerPixel(pf), {100, 100}, pf);
    AImageView view_small(data, 100 * ::bytesPerPixel(pf), {50, 50}, pf);

    ASSERT_EQ(view_big.get({40, 40}), view_small.get({40, 40}));
    // [image_view_stride]
}

TEST(ImageView, CroppedRespectsStride) {
    APixelFormat pf = APixelFormat::R8G8B8A8_UNORM;
    std::vector<uint8_t> some_bytes(100 * 100 * ::bytesPerPixel(pf));
    for (size_t i = 0; i < some_bytes.size(); ++i) {
        some_bytes[i] = static_cast<uint8_t>(i * 7 + i / 400);
    }
    AByteBufferView data(reinterpret_cast<const char*>(some_bytes.data()), some_bytes.size());

    // a 50x50 window into a 100x100 buffer: rows are padded with the other half of the original row
    AImageView window(data, 100 * ::bytesPerPixel(pf), {50, 50}, pf);
    auto cropped = window.cropped({10, 20}, {5, 5});

    for (unsigned y = 0; y < 5; ++y) {
        for (unsigned x = 0; x < 5; ++x) {
            ASSERT_EQ(cropped.get({x, y}), window.get({10 + x, 20 + y})) << x << "," << y;
        }
    }
}
