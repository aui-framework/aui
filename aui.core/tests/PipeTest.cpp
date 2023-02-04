// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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


#include <gtest/gtest.h>
#include "AUI/IO/ADynamicPipe.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Util/ARandom.h"
#include "AUI/IO/AByteBufferInputStream.h"


TEST(Pipe, DynamicBasic) {
    ADynamicPipe pipe;
    AString in = "hello world!", out;
    pipe << aui::serialize_sized(in);
    pipe >> aui::serialize_sized(out);
    EXPECT_EQ(in, out);
    EXPECT_ANY_THROW(pipe >> aui::serialize_sized(out));
}


TEST(Pipe, DynamicComplex) {
    ARandom r;
    for (int bufferSize = 1'000; bufferSize <= 1'000'000; bufferSize *= 10) {
        AByteBuffer input = r.nextBytes(bufferSize);

        for (int readerSize = 1'000; readerSize <= 1'000'000; readerSize *= 10) {
            for (int writerSize = 1'000; writerSize <= 1'000'000; writerSize *= 10) {
                AByteBufferInputStream is(input);
                ADynamicPipe pipe;

                AByteBuffer tmp;
                tmp.resize(writerSize);
                for (;;) {
                    auto r = is.read(tmp.data(), tmp.size());
                    if (r == 0) break;
                    pipe << tmp.slice(0, r);
                }

                AByteBuffer out;
                tmp.resize(readerSize);
                for (;;) {
                    auto r = pipe.read(tmp.data(), tmp.size());
                    if (r == 0) break;
                    out << tmp.slice(0, r);
                }
                ASSERT_EQ(input, out);
            }
        }
    }
}
