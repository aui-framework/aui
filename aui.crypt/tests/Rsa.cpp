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
