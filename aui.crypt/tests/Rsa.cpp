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
    AString s = AString(r->getPrivateKeyPEM(), AStringEncoding::LATIN1);
    ASSERT_TRUE(s.startsWith("-----BEGIN RSA PRIVATE KEY-----"));
    ASSERT_TRUE(s.endsWith("-----END RSA PRIVATE KEY-----\n"));
}

TEST(Rsa, PEM_PublicWrite) {
    auto r = ARsa::generate();
    AString s = AString(r->getPublicKeyPEM(), AStringEncoding::LATIN1);

    ASSERT_TRUE(s.startsWith("-----BEGIN PUBLIC KEY-----") || s.startsWith("-----BEGIN RSA PUBLIC KEY-----"));
    ASSERT_TRUE(s.endsWith("-----END PUBLIC KEY-----\n") || s.endsWith("-----END RSA PUBLIC KEY-----\n"));
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
