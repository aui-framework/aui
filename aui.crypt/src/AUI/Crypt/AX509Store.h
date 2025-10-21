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
#include "AUI/Rsa.h"
#include "AUI/Common/AByteBuffer.h"
#include "AX509.h"

/**
 * @brief x509 certificate store object.
 * @ingroup crypt
 */
class API_AUI_CRYPT AX509Store: public aui::noncopyable
{
private:
    void* mX509Store;
    ADeque<_<AX509>> mCertificates;

public:
    AX509Store();
    ~AX509Store();

    bool validate(_<AX509> cert);
    void addCert(_<AX509> cert);
};
