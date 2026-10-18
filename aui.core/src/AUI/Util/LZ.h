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

#include "AUI/Core.h"
#include "AUI/Common/AException.h"
#include "AUI/IO/IInputStream.h"

#include <AUI/IO/APath.h>
#include <AUI/IO/ISeekableInputStream.h>

class AByteBuffer;

class AByteBufferView;

/**
 * @brief Exception thrown by `aui::zlib` functions.
 * @ingroup io
 */
class AZLibException : public AException {
public:
    AZLibException() {}

    AZLibException(const AString& message) : AException(message) {}
};

namespace aui::zlib {
/**
 * @brief Compresses `b` with zlib (deflate) and appends the result to `dst`.
 * @ingroup io
 * @details
 * <!-- aui:snippet aui.core/tests/ZlibTest.cpp core_zlib -->
 */
void API_AUI_CORE compress(AByteBufferView b, AByteBuffer& dst);

/**
 * @brief Decompresses zlib data `b` and appends the result to `dst`.
 * @ingroup io
 * @details
 * Throws [AZLibException] if the data is corrupted.
 */
void API_AUI_CORE decompress(AByteBufferView b, AByteBuffer& dst);

/**
 * @brief Decompresses zlib data `b` lazily, as the returned stream is read.
 * @ingroup io
 * @details
 * Useful if the decompressed data is large and you process it in chunks. `b` must outlive the stream.
 */
_unique<IInputStream> API_AUI_CORE decompressToStream(AByteBufferView b);

}   // namespace aui::zlib
