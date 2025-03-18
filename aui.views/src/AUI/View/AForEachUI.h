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

#include "AViewContainer.h"
#include <AUI/Model/IListModel.h>
#include <functional>
#include <AUI/Model/AListModelAdapter.h>
#include <AUI/Util/ADataBinding.h>
#include <AUI/Platform/AWindow.h>

class API_AUI_VIEWS AForEachUIBase : public AViewContainerBase {
public:
    AForEachUIBase() {}

protected:
    void applyGeometryToChildren() override { AViewContainerBase::applyGeometryToChildren(); }

    void setModelImpl(_<IListModel<_<AView>>> model);

private:
    _<IListModel<_<AView>>> mViewsModel;
};

namespace aui::detail {
template <typename super, aui::derived_from<ALayout> Layout>
struct ForEachUI {
    template <typename Model>
    class Impl : public super {
    public:
        using T = std::decay_t<decltype(*std::begin(std::declval<Model>()))>;
        using Factory = std::function<_<AView>(const T& t)>;

        Impl(Model&& list) : mDataModel(list) { this->setLayout(std::make_unique<Layout>()); }

        void operator-(Factory f) {
            mFactory = std::move(f);
            updateUnderlyingModel();
        }

        [[nodiscard]]
        const Model& model() const noexcept {
            return mDataModel;
        }

    private:
        Model mDataModel;
        Factory mFactory;

        void updateUnderlyingModel() {
            if (!mFactory) {
                return;
            }

            //        this->setModelImpl(AModels::adapt<_<AView>>(mDataModel, [this](const T& t) { return mFactory(t);
            //        }));
        }
    };

    template <typename Model>
    static _<Impl<Model>> make(Model&& model) {
        return _new<Impl<Model>>(std::forward<Model>(model));
    }
};
}   // namespace aui::detail

#define AUI_DECLARATIVE_FOR_EX(value, model, layout, ...)             \
    aui::ptr::manage(new aui::detail::ForEachUI<AForEachUIBase, layout>::make(model)) - \
        [__VA_ARGS__](const std::decay_t<decltype(model)>::value_type& value) -> _<AView>
#define AUI_DECLARATIVE_FOR(value, model, layout) AUI_DECLARATIVE_FOR_EX(value, model, layout, =)
