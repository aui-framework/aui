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


#include <random>
#include "IInputStream.h"

/**
 * @brief An input stream that generates random bytes.
 * @ingroup io
 */
class API_AUI_CORE ARandomInputStream: public IInputStream {
private:
    std::mt19937 mRandomEngine;

public:
    explicit ARandomInputStream(size_t seed): mRandomEngine(seed) {}
    ~ARandomInputStream() override = default;

    size_t read(char* dst, size_t size) override;
};


