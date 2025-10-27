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

#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Util/APimpl.h>
#include <AUI/api.h>

/**
 * @brief Stores public and/or private RSA key.
 * @ingroup crypt
 */
class API_AUI_CRYPT ARsa : public aui::noncopyable {
private:
    class ARsaPrivate;
    aui::fast_pimpl<ARsaPrivate, 1192> mPrivate;

    ARsa();

public:
    ~ARsa();

    AByteBuffer encrypt(AByteBufferView in);
    AByteBuffer decrypt(AByteBufferView in);

    size_t getKeyLength() const;

    AByteBuffer getPrivateKeyPEM() const;
    AByteBuffer getPublicKeyPEM() const;

    static _<ARsa> generate(int bits = 0x800);
    static _<ARsa> fromPrivateKeyPEM(AByteBufferView buffer);
    static _<ARsa> fromPublicKeyPEM(AByteBufferView buffer);
};
