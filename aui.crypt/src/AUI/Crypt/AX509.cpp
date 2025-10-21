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

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <AUI/Common/AException.h>

AX509::AX509(void* x509): mX509(x509)
{
}

AX509::~AX509()
{
    X509_free((X509*)mX509);
}

bool AX509::isCA() const
{
    return X509_check_ca((X509*)mX509);
}

bool AX509::checkHost(const AString& name) const
{
    auto x = name.toStdString();
    return X509_check_host((X509*)mX509, x.data(), x.length(), 0, nullptr);
}

bool AX509::checkTrust() const
{
    return X509_trusted((X509*)mX509);
}

_<AX509> AX509::fromPEM(AByteBufferView buffer)
{
    BIO* inputBuffer = BIO_new_mem_buf(buffer.data(), buffer.size());
    X509* rsa = nullptr;
    PEM_read_bio_X509(inputBuffer, (X509**)&rsa, nullptr, nullptr);
    BIO_free(inputBuffer);

    if (rsa == nullptr)
    	throw AException("Could not create x509 certificate");

    return aui::ptr::manage_shared(new AX509(rsa));
}
