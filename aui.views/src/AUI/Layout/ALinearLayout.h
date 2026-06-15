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

#include <AUI/Common/AObject.h>
#include "ALayout.h"
#include <AUI/Traits/concepts.h>

class AView;

namespace aui::detail {
    template<aui::convertible_to<AArc<AView>> Storage = AArc<AView>>
    class LinearLayoutImpl: public ALayout {
    public:
        void removeView(aui::no_escape<AView> view, size_t index) override {
            if constexpr (std::is_same_v<Storage, AArc<AView>>) {
                AUI_ASSERT(mViews[index].get() == view.ptr());
            }
            mViews.removeAt(index);
        }

        AVector<AArc<AView>> getAllViews() override {
            return { mViews.begin(), mViews.end() };
        }

    protected:
        void addViewBasicImpl(Storage view, AOptional<size_t> index) {
            auto at = mViews.end();
            if (index) {
                at = mViews.begin() + *index;
            }
            mViews.insert(at, std::move(view));
        }

    protected:
        AVector<Storage> mViews;
    };

}

/**
 * @brief Implements addView/removeView/getAllViews and protected mViews field for Vertical, Horizontal and Stacked layouts.
 * @tparam Storage optional storage type. See details for further info.
 * @details
 * The Storage type is `AArc<AView>` by default. If you specify your own type, you would have to implement addView
 * to fill your custom type with data, you may use addViewBasicImpl then.
 *
 */
template<aui::convertible_to<AArc<AView>> Storage = AArc<AView>>
class ALinearLayout: public aui::detail::LinearLayoutImpl<Storage> {};

template<>
class ALinearLayout<AArc<AView>>: public aui::detail::LinearLayoutImpl<AArc<AView>> {
public:
    void addView(const AArc<AView>& view, AOptional<size_t> index) override {
        addViewBasicImpl(view, index);
    }
};
