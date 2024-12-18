/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AUI/Core.h"
#include "AUI/Common/AException.h"
#include "AUI/IO/IInputStream.h"

class AByteBuffer;

class AByteBufferView;

class AZLibException : public AException {
public:
    AZLibException() {
    }

    AZLibException(const AString& message)
            : AException(message) {
    }
};

namespace aui::zlib {
    void API_AUI_CORE compress(AByteBufferView b, AByteBuffer& dst);
    void API_AUI_CORE decompress(AByteBufferView b, AByteBuffer& dst);
    _unique<IInputStream> API_AUI_CORE decompressToStream(AByteBufferView b);
}
