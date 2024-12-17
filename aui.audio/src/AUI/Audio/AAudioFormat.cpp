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

#include "AAudioFormat.h"
#include "AUI/Audio/ACompileTimeSoundResampler.h"

std::uint32_t AAudioFormat::bitsPerSample() const {
    switch (sampleFormat) {
        case ASampleFormat::I16: return aui::audio::impl::sample_type<ASampleFormat::I16>::size_bits;
        case ASampleFormat::I24: return aui::audio::impl::sample_type<ASampleFormat::I24>::size_bits;
    }
    throw AException("unable to find appropriate sample format");
}
