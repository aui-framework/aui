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

#pragma once

#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents custom-rendered background effect.
     * @ingroup ass
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

            void renderFor(AView* view) override;

            bool isNone() override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };

    }
}