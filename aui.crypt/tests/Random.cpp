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
#include <AUI/Crypt/Random.h>

TEST(Random, SafeRandomSizeAndEntropy) {
    auto a = ACrypto::safeRandom();
    auto b = ACrypto::safeRandom();
    ASSERT_EQ(a->size(), 0x1000);
    ASSERT_EQ(b->size(), 0x1000);

    // two independent draws of 4 KiB must not match
    EXPECT_NE(0, std::memcmp(a->data(), b->data(), a->size()));

    // the buffer must not be (nearly) constant
    std::array<size_t, 256> histogram {};
    for (size_t i = 0; i < a->size(); ++i) {
        ++histogram[static_cast<uint8_t>(a->data()[i])];
    }
    EXPECT_GT(std::count_if(histogram.begin(), histogram.end(), [](size_t n) { return n > 0; }), 200);
}
