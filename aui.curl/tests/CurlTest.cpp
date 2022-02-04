//
// Created by Alex2772 on 2/4/2022.
//

#include <gtest/gtest.h>
#include "AUI/Curl/ACurl.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/IO/AFileInputStream.h"

TEST(CurlTest, ToByteBuffer) {
    AByteBuffer buffer = ACurl::Builder("https://github.com").toByteBuffer();
    ASSERT_TRUE(AString::fromUtf8(buffer).contains("DOCTYPE"));
}

TEST(CurlTest, ToStream) {
    {
        ACurl a(ACurl::Builder("https://github.com").withOutputStream(_new<AFileOutputStream>("temp.html")));
        a.run();
    }
    auto buffer = AByteBuffer::fromStream(AFileInputStream("temp.html"));
    ASSERT_TRUE(AString::fromUtf8(buffer).contains("DOCTYPE"));
}

TEST(CurlTest, Fail) {
    ACurl a(ACurl::Builder("https://snonejklgnjkwernjkg3ernjknerjk.sfjgsjknsdgjknsdgjknjksdgnjkzdg").withOutputStream(_new<AFileOutputStream>("temp.html")));
    EXPECT_THROW(a.run(), ACurl::Exception);
}
