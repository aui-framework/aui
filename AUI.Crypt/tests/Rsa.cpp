/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 31.08.2020.
//

#include <boost/test/unit_test.hpp>
#include <AUI/Crypt/ARsa.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Common/AString.h>

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE(Rsa)

    BOOST_AUTO_TEST_CASE(EncryptDecrypt) {
        unsigned bits = 512;
        for (uint32_t i = 0; i < 4; ++i, bits <<= 1) {
            auto r = ARsa::generate();
            for (auto size : {10, 1000, 5000}) {
                auto someData = ARandom().nextBytes(size);

                auto encrypted = r->encrypt(someData);
                auto decrypted = r->decrypt(encrypted);
                BOOST_CHECK_EQUAL(*decrypted, *someData);
            }
        }

    }

    BOOST_AUTO_TEST_CASE(PEM_PrivateWrite) {
        auto r = ARsa::generate();
        AString s = AString::fromLatin1(r->getPrivateKeyPEM());
        BOOST_TEST(s.startsWith("-----BEGIN RSA PRIVATE KEY-----"));
        BOOST_TEST(s.endsWith("-----END RSA PRIVATE KEY-----\n"));
    }

    BOOST_AUTO_TEST_CASE(PEM_PublicWrite) {
        auto r = ARsa::generate();
        AString s = AString::fromLatin1(r->getPublicKeyPEM());

        BOOST_TEST(s.startsWith("-----BEGIN RSA PUBLIC KEY-----"));
        BOOST_TEST(s.endsWith("-----END RSA PUBLIC KEY-----\n"));
    }

    BOOST_AUTO_TEST_CASE(PEM_PrivateRead) {
        auto both = ARsa::generate();
        auto priv = ARsa::fromPrivateKeyPEM(both->getPrivateKeyPEM());

        auto someData = ARandom().nextBytes(10000);
        auto result = priv->decrypt(both->encrypt(someData));

        BOOST_CHECK_EQUAL(*someData, *result);

    }

    BOOST_AUTO_TEST_CASE(PEM_PublicRead) {
        auto both = ARsa::generate();
        auto pub = ARsa::fromPublicKeyPEM(both->getPublicKeyPEM());

        auto someData = ARandom().nextBytes(10000);
        auto result = both->decrypt(pub->encrypt(someData));

        BOOST_CHECK_EQUAL(*someData, *result);

    }

BOOST_AUTO_TEST_SUITE_END()