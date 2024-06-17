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
#include "IInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/SharedPtr.h"


/**
 * @brief Converts a AByteBuffer to an IInputStream.
 * @ingroup io
 */
class API_AUI_CORE AByteBufferInputStream final: public IInputStream
{
private:
    const char* mCurrent;
    const char* mEnd;


public:
	AByteBufferInputStream(AByteBufferView buffer)
		: mCurrent(buffer.data()), mEnd(buffer.data() + buffer.size())
	{
	}

	size_t read(char* dst, size_t size) override;

    size_t available() const {
        return mEnd - mCurrent;
    }
};
