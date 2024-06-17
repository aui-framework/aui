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
// Created by alex2 on 3.12.2020.
//

#pragma once

#include "AUI/Render/ARender.h"
#include "IProperty.h"

namespace ass {

    /**
     * @brief Controls background texture cropping by exact UV coordinates. Useful for texture atlases.
     * @ingroup ass
     */
    struct BackgroundCropping {
        unset_wrap<glm::vec2> offset;
        unset_wrap<glm::vec2> size;

        BackgroundCropping() noexcept = default;
        BackgroundCropping(const BackgroundCropping&) noexcept = default;
        BackgroundCropping(const glm::vec2& offset, const unset_wrap<glm::vec2>& size) noexcept:
            offset(offset), size(size) {}

        static const BackgroundCropping H4_1;
        static const BackgroundCropping H4_2;
        static const BackgroundCropping H4_3;
        static const BackgroundCropping H4_4;
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BackgroundCropping>: IPropertyBase {
        private:
            BackgroundCropping mInfo;

        public:
            Property(const BackgroundCropping& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };

    }
}

inline const ass::BackgroundCropping ass::BackgroundCropping::H4_1 = { glm::vec2{0.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };
inline const ass::BackgroundCropping ass::BackgroundCropping::H4_2 = { glm::vec2{1.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };
inline const ass::BackgroundCropping ass::BackgroundCropping::H4_3 = { glm::vec2{2.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };
inline const ass::BackgroundCropping ass::BackgroundCropping::H4_4 = { glm::vec2{3.f / 4.f, 0.f}, glm::vec2{1.f / 4.f, 1.f} };