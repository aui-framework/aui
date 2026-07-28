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

//
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Common/AString.h>
#include <AUI/Crypt/AHash.h>
#include <AUI/IO/AByteBufferInputStream.h>


TEST(Hash, Md5) {
    ASSERT_EQ(AHash::md5(AByteBuffer::fromString("govno")).toHexString(), "b3575f222f7b768c25160b879699118b");

    AByteBuffer buffer = AByteBuffer::fromString("govno");
    ASSERT_EQ(AHash::md5(_new<AByteBufferInputStream>(buffer)).toHexString(), "b3575f222f7b768c25160b879699118b");
}
TEST(Hash, Sha512) {
    ASSERT_EQ(AHash::sha512(AByteBuffer::fromString("govno")).toHexString(), "b7cbd9e15895669db8806632dc00894b4551e172220bdadedbae7005291e1a1586a172012e1319fbff968760bcc13d96015acdff8c115b8f1e3e7b421126bb03");

    AByteBuffer buffer = AByteBuffer::fromString("sdfzsrsrhsrhfxbuihusebrvjmsdfbvhsrhvbhfsvbhbhlsdbhjbsdhbdfhbhlefbhlABHJ");

    ASSERT_EQ(AHash::sha512(_new<AByteBufferInputStream>(buffer)).toHexString(), "b45f03fb7627749aa177814526a23547436df87b3233393d86586b4ecd043327b94a67f7d1ee56ae43faf8b8290fd0cfb2b11d46134b331c4a1ff1f2da6a3ca8");
}
