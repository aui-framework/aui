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

#include "AUI/Audio/ISoundInputStream.h"

/**
 * @brief Sound stream that outputs raw samples
 * @ingroup audio
 */
class ARawSoundStream : public ISoundInputStream {
public:
    ARawSoundStream(AAudioFormat format, _<IInputStream>);

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

private:
    AAudioFormat mFormat;
    _<IInputStream> mStream;
};