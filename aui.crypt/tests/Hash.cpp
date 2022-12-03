// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Crypt/ARsa.h>
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
