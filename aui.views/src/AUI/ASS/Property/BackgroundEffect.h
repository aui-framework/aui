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

#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents custom-rendered background effect.
     * @ingroup ass_properties
     * @see background_effect
     */
    struct BackgroundEffect {
        AVector<_<IBackgroundEffect>> mEffects;

        BackgroundEffect(std::nullptr_t) {}

        template<typename... Args>
        BackgroundEffect(Args&&... args): mEffects({ _new<Args>(std::move(args))... }) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BackgroundEffect>: IPropertyBase {
        private:
            BackgroundEffect mInfo;

        public:
            Property(const BackgroundEffect& info) : mInfo(info) {

            }

            void renderFor(AView* view, const ARenderContext& ctx) override;

            bool isNone() override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };

    }
}