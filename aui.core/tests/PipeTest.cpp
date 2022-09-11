/*
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
