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

#include "ATlsIdentity.h"

#include <ctime>

#include <mbedtls/ecp.h>
#include <mbedtls/pk.h>
#include <mbedtls/psa_util.h>
#include <mbedtls/x509_crt.h>
#include <psa/crypto.h>

#include "AHash.h"
#include "ATlsSession.h"
#include "TlsInternal.h"

namespace {
void check(int code, const char* what) {
    if (code != 0) {
        throw ATlsException(AString(what) + ": " + aui::tls::describe(code));
    }
}

std::string timestamp(std::chrono::system_clock::time_point t) {
    const auto time = std::chrono::system_clock::to_time_t(t);
    std::tm utc {};
#if AUI_PLATFORM_WIN
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &utc);
    return buffer;
}
}   // namespace

ATlsIdentity::ATlsIdentity(std::unique_ptr<Impl> impl) : mImpl(std::move(impl)) {}
ATlsIdentity::~ATlsIdentity() = default;

_<ATlsIdentity> ATlsIdentity::generateSelfSigned(const AString& commonName, std::chrono::hours validity) {
    aui::tls::init();
    auto impl = std::make_unique<Impl>();

    check(mbedtls_pk_setup(&impl->key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)), "unable to set up the key");
    check(mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, mbedtls_pk_ec(impl->key), aui::tls::random, nullptr),
          "unable to generate the key");

    mbedtls_x509write_cert writer;
    mbedtls_x509write_crt_init(&writer);
    struct Free {
        mbedtls_x509write_cert& w;
        ~Free() { mbedtls_x509write_crt_free(&w); }
    } free { writer };

    const auto subject = "CN=" + commonName.toStdString();
    const auto now = std::chrono::system_clock::now();
    unsigned char serial[16];
    check(aui::tls::random(nullptr, serial, sizeof(serial)), "unable to generate the serial");
    serial[0] &= 0x7f;   // positive
    serial[0] |= 0x01;   // and not zero

    mbedtls_x509write_crt_set_version(&writer, MBEDTLS_X509_CRT_VERSION_3);
    mbedtls_x509write_crt_set_md_alg(&writer, MBEDTLS_MD_SHA256);
    check(mbedtls_x509write_crt_set_serial_raw(&writer, serial, sizeof(serial)), "unable to set the serial");
    check(mbedtls_x509write_crt_set_subject_name(&writer, subject.c_str()), "unable to set the subject");
    check(mbedtls_x509write_crt_set_issuer_name(&writer, subject.c_str()), "unable to set the issuer");
    check(mbedtls_x509write_crt_set_validity(&writer, timestamp(now - std::chrono::hours(24)).c_str(),
                                             timestamp(now + validity).c_str()),
          "unable to set the validity");
    mbedtls_x509write_crt_set_subject_key(&writer, &impl->key);
    mbedtls_x509write_crt_set_issuer_key(&writer, &impl->key);

    unsigned char der[1024];
    const int length = mbedtls_x509write_crt_der(&writer, der, sizeof(der), aui::tls::random, nullptr);
    if (length < 0) {
        check(length, "unable to write the certificate");
    }
    // written to the end of the buffer
    const unsigned char* start = der + sizeof(der) - length;
    check(mbedtls_x509_crt_parse_der(&impl->certificate, start, static_cast<std::size_t>(length)),
          "unable to parse the certificate");
    impl->fingerprint = AHash::sha256(AByteBufferView(reinterpret_cast<const char*>(start), length));
    return aui::ptr::manage_shared(new ATlsIdentity(std::move(impl)));
}

const AByteBuffer& ATlsIdentity::fingerprintSha256() const noexcept { return mImpl->fingerprint; }

AString ATlsIdentity::fingerprintHex() const { return toHex(mImpl->fingerprint); }

AString ATlsIdentity::toHex(AByteBufferView fingerprint) {
    static constexpr char HEX[] = "0123456789abcdef";
    std::string result;
    for (std::size_t i = 0; i < fingerprint.size(); ++i) {
        const auto byte = static_cast<std::uint8_t>(fingerprint.data()[i]);
        result.push_back(HEX[byte >> 4]);
        result.push_back(HEX[byte & 0xf]);
    }
    return AString(result);
}
