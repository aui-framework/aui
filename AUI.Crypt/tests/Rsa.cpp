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