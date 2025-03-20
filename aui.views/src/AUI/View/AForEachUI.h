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

#pragma once

#include <range/v3/view/transform.hpp>

#include "AViewContainer.h"
#include <AUI/Model/IListModel.h>
#include <functional>
#include <AUI/Model/AListModelAdapter.h>
#include <AUI/Util/ADataBinding.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Traits/dyn_range.h>

class API_AUI_VIEWS AForEachUIBase: public AViewContainerBase {
public:
    using List = aui::dyn_range<_<AView>>;
    AForEachUIBase() {}

protected:
    void applyGeometryToChildren() override {
        AViewContainerBase::applyGeometryToChildren();
    }

    void setModelImpl(List model);

private:
    List mViewsModel;
};

template<typename T, typename Layout, typename super = AForEachUIBase>
class AForEachUI: public super {
public:
    using List = aui::dyn_range<_<AView>>;
    using Factory = std::function<_<AView>(const T& value)>;

    AForEachUI() {
        this->setLayout(std::make_unique<Layout>());
    }

    template<ranges::range Rng>
    AForEachUI(Rng&& rng) {
        this->setLayout(std::make_unique<Layout>());
        this->setModel(std::forward<Rng>(rng));
    }

    template<ranges::range Rng>
    requires requires (Rng&& rng) { { *ranges::begin(rng) } -> aui::convertible_to<T>; }
    void setModel(Rng && rng) {
        mDataModel = rng | ranges::views::transform([this](const T& t) { return mFactory(t); });
        updateUnderlyingModel();
    }

    void operator-(Factory f) {
        mFactory = std::move(f);
        updateUnderlyingModel();
    }

    [[nodiscard]]
    const List& model() const noexcept {
        return mDataModel;
    }

private:
    List mDataModel;
    Factory mFactory;

    void updateUnderlyingModel() {
        if (!mFactory) {
            return;
        }

        this->setModelImpl(mDataModel);
    }

};

namespace aui::detail {
template<typename Base /* AForEachUIBase */, typename Layout, ranges::range Rng>
auto makeForEach(Rng&& rng) {
    using T = std::decay_t<decltype(*ranges::begin(rng))>;
    return _new<AForEachUI<T, Layout, Base>>(std::forward<Rng>(rng));
}
}

#ifndef __clang__
#define AUI_DECLARATIVE_FOR_EX(value, model, layout, ...) aui::detail::makeForEach<AForEachUIBase, layout>(model) - [__VA_ARGS__](const auto& value) -> _<AView>
#define AUI_DECLARATIVE_FOR(value, model, layout) AUI_DECLARATIVE_FOR_EX(value, model, layout, =)
#endif
