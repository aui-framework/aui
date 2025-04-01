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
#include "AUI/Common/AString.h"
#include "IInputStream.h"
#include "IOutputStream.h"

/**
 * @brief A buffer input/output stream based on std::string.
 * @ingroup io
 */
class API_AUI_CORE AStringStream: public IInputStream, public IOutputStream
{
private:
	std::string mString;
	size_t mReadPos = 0;

public:
    AStringStream();
    explicit AStringStream(std::string string);
	explicit AStringStream(const AString& string);
	virtual ~AStringStream() = default;

	void seekRead(size_t position);

	size_t read(char* dst, size_t size) override;
    void write(const char *src, size_t size) override;

    [[nodiscard]]
    const std::string& str() const noexcept {
        return mString;
    }
};
