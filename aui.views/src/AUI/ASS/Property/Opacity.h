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


#include <AUI/Util/AMetric.h>
#include <AUI/Traits/values.h>
#include <AUI/Common/IStringable.h>
#include "IProperty.h"

#if defined(FMT_VERSION) && (FMT_VERSION >= 100000)
template <> struct fmt::formatter<aui::float_within_0_1> : fmt::formatter<float> {
    auto format(const aui::float_within_0_1& v, fmt::format_context& ctx) const {
        return fmt::formatter<float>::format(static_cast<float>(v), ctx);
    }
};
#endif

namespace ass {

    /**
     * @brief Controls the opacity of AView.
     * @ingroup ass_properties
     */
    struct Opacity {
        aui::float_within_0_1 opacity;

        explicit Opacity(float opacity) : opacity(opacity) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<Opacity>: IPropertyBase, IStringable {
        private:
            Opacity mInfo;

        public:
            Property(const Opacity& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }

            AString toString() const override {
                return "Opacity({})"_format(mInfo.opacity);
            }
        };
    }
}