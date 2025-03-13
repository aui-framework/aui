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
#include <AUI/Model/AListModelObserver.h>
#include <functional>
#include <AUI/Util/ADataBinding.h>
#include <AUI/Platform/AWindow.h>

template<typename T, typename Layout>
class AForEachUIBase: public AViewContainerBase, public AListModelObserver<T>::IListModelListener {
public:
    using List = _<IListModel<T>>;
    using Factory = std::function<_<AView>(const T& value, size_t index)>;
private:
    _<AListModelObserver<T>> mObserver;

    Factory mFactory;

public:
    AForEachUIBase(List list):
        mObserver(_new<AListModelObserver<T>>(this)) {
        setLayout(std::make_unique<Layout>());
        setModel(std::move(list));
    }

    void setModel(List list) {
        mObserver->setModel(std::move(list));
    }

    void insertItem(size_t at, const T& value) override {
        if (!mFactory) {
            return;
        }
        addView(at, mFactory(value, at));
    }

    void updateItem(size_t at, const T& value) override {
        // TODO optimize
        removeView(at);
        addView(at, mFactory(value, at));
    }

    void removeItem(size_t at) override {
        removeView(at);
    }

    void onDataCountChanged() override {
        markMinContentSizeInvalid();
    }

    void onDataChanged() override {
        markMinContentSizeInvalid();
    }

    void operator-(const Factory& f) {
        mFactory = f;
        setModel(mObserver->getModel());
    }

protected:
    void applyGeometryToChildren() override {
        AViewContainerBase::applyGeometryToChildren();
    }

public:
    int getContentMinimumWidth() override {
        return AViewContainerBase::getContentMinimumWidth();
    }

    int getContentMinimumHeight() override {
        return AViewContainerBase::getContentMinimumHeight();
    }
};

#define AUI_DECLARATIVE_FOR_BASE_EX(value, model, layout, ...) _new<AForEachUI<std::decay_t<decltype(model)>::stored_t::stored_t, layout>>(model) - [__VA_ARGS__](const std::decay_t<decltype(model)>::stored_t::stored_t& value, size_t index) -> _<AView>
#define AUI_DECLARATIVE_FOR_BASE(value, model, layout) AUI_DECLARATIVE_FOR_BASE_EX(value, model, layout, =)
