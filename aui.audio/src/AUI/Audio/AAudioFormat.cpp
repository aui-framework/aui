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

#include "AAudioFormat.h"

namespace aui::audio {
void convertSampleFormat(ASampleFormat inF, ASampleFormat outF, const char* inB, char* outB, size_t samples) {
    if (inF == outF) {
        size_t bytesToCopy = samples * bytesPerSample(inF);
        std::memcpy(outB, inB, bytesToCopy);
        return;
    }

    for (size_t i = 0; i < samples; ++i) {
        float normalizedSample = 0.0f;

        switch (inF) {
            case ASampleFormat::I16: {
                int16_t sample = *reinterpret_cast<const int16_t*>(inB + i * 2);
                normalizedSample = static_cast<float>(sample) / 32768.0f;
                break;
            }
            case ASampleFormat::I24: {
                int32_t sample = 0;
                std::memcpy(&sample, inB + i * 3, 3);
                if (sample & 0x800000) {
                    sample |= 0xFF000000;
                }
                normalizedSample = static_cast<float>(sample) / 8388608.0f;
                break;
            }
            case ASampleFormat::I32: {
                int32_t sample = *reinterpret_cast<const int32_t*>(inB + i * 4);
                normalizedSample = static_cast<float>(sample) / 2147483648.0f;
                break;
            }
            case ASampleFormat::F32: {
                normalizedSample = *reinterpret_cast<const float*>(inB + i * 4);
                break;
            }
        }

        normalizedSample = std::clamp(normalizedSample, -1.0f, 1.0f);

        switch (outF) {
            case ASampleFormat::I16: {
                int16_t sample = static_cast<int16_t>(normalizedSample * 32767.0f);
                *reinterpret_cast<int16_t*>(outB + i * 2) = sample;
                break;
            }
            case ASampleFormat::I24: {
                int32_t sample = static_cast<int32_t>(normalizedSample * 8388607.0f);
                std::memcpy(outB + i * 3, &sample, 3);
                break;
            }
            case ASampleFormat::I32: {
                int32_t sample = static_cast<int32_t>(normalizedSample * 2147483647.0f);
                *reinterpret_cast<int32_t*>(outB + i * 4) = sample;
                break;
            }
            case ASampleFormat::F32: {
                *reinterpret_cast<float*>(outB + i * 4) = normalizedSample;
                break;
            }
        }
    }
}
}
