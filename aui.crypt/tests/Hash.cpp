/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
