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

#pragma once

#include <AUI/Common/AByteBuffer.h>
#include <AUI/Util/APimpl.h>
#include <AUI/api.h>

typedef struct mbedtls_x509_crt mbedtls_x509_crt;

/**
 * @brief x509 certificate object.
 * @ingroup crypt
 */
class API_AUI_CRYPT AX509: public aui::noncopyable
{
    friend class AX509Store;
private:
    aui::fast_pimpl<mbedtls_x509_crt, 744> mX509;

    explicit AX509(aui::fast_pimpl<mbedtls_x509_crt, 744> x509);

public:
    ~AX509();

    bool isCA() const;
    bool checkHost(const AString& name) const;
    bool checkTrust() const;

    static _<AX509> fromPEM(AByteBufferView buffer);
};
