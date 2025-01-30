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

#if AUI_PLATFORM_LINUX
#include "AUI/Platform/Pipe.h"
TEST(Pipe, UnixPipe) {
    Pipe pipe;
    AString in = "hello world!", out;
    pipe << aui::serialize_sized(in);
    pipe >> aui::serialize_sized(out);
    EXPECT_EQ(in, out);
}
#endif

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
