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

class API_AUI_VIEWS AForEachUIBase: public AViewContainerBase {
public:
    AForEachUIBase() {}

protected:
    void applyGeometryToChildren() override {
        AViewContainerBase::applyGeometryToChildren();
    }

    void setModelImpl(_<IListModel<_<AView>>> model);

private:
    _<IListModel<_<AView>>> mViewsModel;
};

template<typename T, typename Layout, typename super = AForEachUIBase>
class AForEachUI: public super {
public:
    using List = _<IListModel<T>>;
    using Factory = std::function<_<AView>(const T& value)>;

    AForEachUI(List list): mDataModel(std::move(list)) {
        this->setLayout(std::make_unique<Layout>());
    }

    void setModel(List list) {
        mDataModel = std::move(list);
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
        if (!mDataModel) {
            return;
        }
        if (!mFactory) {
            return;
        }

        this->setModelImpl(AModels::adapt<_<AView>>(mDataModel, [this](const T& t) { return mFactory(t); }));
    }

};

#define AUI_DECLARATIVE_FOR_EX(value, model, layout, ...) _new<AForEachUI<std::decay_t<decltype(model)>::stored_t::stored_t, layout>>(model) - [__VA_ARGS__](const std::decay_t<decltype(model)>::stored_t::stored_t& value) -> _<AView>
#define AUI_DECLARATIVE_FOR(value, model, layout) AUI_DECLARATIVE_FOR_EX(value, model, layout, =)
