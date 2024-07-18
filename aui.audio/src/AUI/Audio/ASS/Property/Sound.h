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

//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "AUI/Audio/IAudioPlayer.h"
#include <AUI/ASS/ASS.h>
#include <AUI/Audio/IAudioPlayer.h>

namespace ass {

    /**
     * @brief Produces sound effect when the style applied.
     * @ingroup ass
     */
    struct Sound {
        /**
         * @brief Player of the sound effect.
         */
        _<IAudioPlayer> sound;
    };


    namespace prop {
        template<>
        struct Property<Sound>: IPropertyBase {
        private:
            Sound mInfo;

        public:
            Property(const Sound& info) : mInfo(info) {

            }

            void applyFor(AView* view) override {
                mInfo.sound->stop();
                mInfo.sound->play();
            }

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}