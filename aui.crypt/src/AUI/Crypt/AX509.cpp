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

#include "AX509.h"

#include <mbedtls/x509_crt.h>
#include <mbedtls/x509.h>
#include <AUI/Common/AException.h>

AX509::AX509(aui::fast_pimpl<mbedtls_x509_crt, 744> x509) : mX509(std::move(x509)) {}

AX509::~AX509()
{
    mbedtls_x509_crt_free(mX509.ptr());
}

bool AX509::isCA() const
{
    return mbedtls_x509_crt_get_ca_istrue(mX509.ptr());
}

static bool dns_name_match(std::string_view pattern, std::string_view name)
{
    if (pattern.empty() || name.empty()) return false;
    if (pattern.size() > 0 && pattern[0] == '*') {
        if (pattern.size() >= 2 && pattern[1] == '.') {
            std::string_view suffix = pattern.substr(1);
            if (name.size() >= suffix.size() &&
                name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0)
                return true;
        }
        return false;
    }
    return pattern == name;
}

bool AX509::checkHost(const AString& name) const
{
    /*auto crt = mX509.ptr();
    std::string host = name.toStdString();

    for (const mbedtls_x509_sequence* cur = &crt->subject_alt_names; cur != nullptr && cur->buf.p != nullptr; cur = cur->next) {
        mbedtls_x509_subject_alternative_name san;
        int ret = mbedtls_x509_parse_subject_alt_name(&cur->buf, &san);
        if (ret != 0) continue;
        if (san.type == MBEDTLS_X509_SAN_DNS_NAME) {
            std::string_view dns((const char*)san.san.dns_name.p, san.san.dns_name.len);
            if (dns_name_match(dns, host)) return true;
        }
    }

    for (const mbedtls_x509_name* cur = &crt->subject; cur != nullptr; cur = cur->next) {
        if (MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &cur->oid) == 0) {
            std::string_view cn((const char*)cur->val.p, cur->val.len);
            if (dns_name_match(cn, host)) return true;
        }
    }

    return false;*/
}

bool AX509::checkTrust() const
{
    unsigned int flags = 0;

    int ret = mbedtls_x509_crt_verify(
        const_cast<mbedtls_x509_crt*>(mX509.ptr()),
        const_cast<mbedtls_x509_crt*>(mX509.ptr()),
        nullptr,
        nullptr,
        &flags,
        nullptr,
        nullptr
    );

    return ret == 0 && flags == 0;
}

_<AX509> AX509::fromPEM(AByteBufferView buffer)
{
    aui::fast_pimpl<mbedtls_x509_crt, 744> crt;
    mbedtls_x509_crt_init(crt.ptr());

    std::vector<unsigned char> pemData(buffer.size() + 1);
    std::memcpy(pemData.data(), buffer.data(), buffer.size());
    pemData[buffer.size()] = '\0';

    int ret = mbedtls_x509_crt_parse(crt.ptr(), pemData.data(), pemData.size());

    if (ret != 0) {
        mbedtls_x509_crt_free(crt.ptr());
        throw AException("Could not create x509 certificate");
    }

    return aui::ptr::manage_shared(new AX509(std::move(crt)));
}
