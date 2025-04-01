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
#include "IInputStream.h"
#include "ISeekableInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/SharedPtr.h"

/**
 * @brief Converts a AByteBuffer to an IInputStream.
 * @ingroup io
 */
class API_AUI_CORE AByteBufferInputStream final : public ISeekableInputStream {
private:
    const char* mBegin;
    const char* mEnd;
    const char* mCurrent;

public:
    AByteBufferInputStream(AByteBufferView buffer) : mBegin(buffer.data()), mEnd(buffer.data() + buffer.size()), mCurrent(mBegin) {}
    ~AByteBufferInputStream() override = default;
    void seek(std::streamoff offset, ASeekDir seekDir) override;
    [[nodiscard]] std::streampos tell() noexcept override;
    bool isEof() override;

    size_t read(char* dst, size_t size) override;

    size_t available() const { return mEnd - mCurrent; }
};
