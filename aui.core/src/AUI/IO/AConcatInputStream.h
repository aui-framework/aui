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
// Created by alex2 on 13.11.2020.
//

#pragma once


#include "IInputStream.h"
#include <AUI/Common/AVector.h>

/**
 * @brief Concatenates multiple sequential input streams into one.
 * @ingroup io
 */
class API_AUI_CORE AConcatInputStream: public IInputStream {
private:
    ADeque<_<IInputStream>> mInputStreams;

public:
    explicit AConcatInputStream(ADeque<_<IInputStream>> inputStreams) noexcept: mInputStreams(std::move(inputStreams)) {

    }

    ~AConcatInputStream() override = default;

    size_t read(char* dst, size_t size) override;
};


