// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
                mInfo.sound->play();
            }

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}