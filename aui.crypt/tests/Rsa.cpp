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



TEST(Rsa, EncryptDecrypt) {
    unsigned bits = 512;
    for (uint32_t i = 0; i < 4; ++i, bits <<= 1) {
        auto r = ARsa::generate();
        for (auto size : {10, 1000, 5000}) {
            auto someData = ARandom().nextBytes(size);

            auto encrypted = r->encrypt(someData);
            auto decrypted = r->decrypt(encrypted);
            ASSERT_EQ(decrypted, someData);
        }
    }

}

TEST(Rsa, PEM_PrivateWrite) {
    auto r = ARsa::generate();
    AString s = AString::fromLatin1(r->getPrivateKeyPEM());
    ASSERT_TRUE(s.startsWith("-----BEGIN RSA PRIVATE KEY-----"));
    ASSERT_TRUE(s.endsWith("-----END RSA PRIVATE KEY-----\n"));
}

TEST(Rsa, PEM_PublicWrite) {
    auto r = ARsa::generate();
    AString s = AString::fromLatin1(r->getPublicKeyPEM());

    ASSERT_TRUE(s.startsWith("-----BEGIN RSA PUBLIC KEY-----"));
    ASSERT_TRUE(s.endsWith("-----END RSA PUBLIC KEY-----\n"));
}

TEST(Rsa, PEM_PrivateRead) {
    auto both = ARsa::generate();
    auto priv = ARsa::fromPrivateKeyPEM(both->getPrivateKeyPEM());

    auto someData = ARandom().nextBytes(10000);
    auto result = priv->decrypt(both->encrypt(someData));

    ASSERT_EQ(someData, result);

}

TEST(Rsa, PEM_PublicRead) {
    auto both = ARsa::generate();
    auto pub = ARsa::fromPublicKeyPEM(both->getPublicKeyPEM());

    auto someData = ARandom().nextBytes(10000);
    auto result = both->decrypt(pub->encrypt(someData));

    ASSERT_EQ(someData, result);

}
