/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Image/AImage.h"
#include "AUI/Util/ABuiltinFiles.h"
#include <range/v3/all.hpp>

const static unsigned char AUI_PACKED_assettarget[] = "\x78\xda\x73\xf2\x35\x63\x66\x00\x03\x33\x20\xd6\x00\x62\x01\x28\x66\x64\x90\x80\x48\x00\xe5\x55\xc5\x20\x18\x06\xd8\x19\xff\x8f\xa2\x51\x34\x62\x11\x00\xf4\x5a\x08\xe8";

TEST(ImageLoader, Bmp) {
    ABuiltinFiles::registerAsset("target.bmp", AUI_PACKED_assettarget, sizeof(AUI_PACKED_assettarget));
    auto image = AImage::fromUrl(":target.bmp");

    ASSERT_TRUE(image != nullptr);
    ASSERT_EQ(image->width(), 16);
    ASSERT_EQ(image->height(), 16);
}
