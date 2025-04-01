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

//
// Created by Alex2772 on 4/18/2022.
//

#include <AUI/IO/AByteBufferInputStream.h>
#include "AByteBufferView.h"

AString AByteBufferView::toHexString() const {
    AString result;
    result.reserve(size() * 2 + 10);
    char buf[8];

    for (size_t i = 0; i < size(); ++i) {
        result += std::string_view(buf, std::distance(std::begin(buf), fmt::format_to(std::begin(buf), "{:02x}", (std::uint8_t)mBuffer[i])));
    }
    return result;
}

_unique<IInputStream> AByteBufferView::toStream() const {
    class ByteBufferStream: public IInputStream {
    private:
        AByteBuffer mData;
        AByteBufferInputStream mIs;

    public:
        ByteBufferStream(AByteBufferView view): mData(view), mIs(mData) {

        }

        size_t read(char* dst, size_t size) override {
            return mIs.read(dst, size);
        }

        ~ByteBufferStream() override = default;
    };

    return std::make_unique<ByteBufferStream>(*this);
}
