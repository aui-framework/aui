// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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


#include "AViewContainer.h"
#include <AUI/Model/IListModel.h>
#include <AUI/Model/AListModelObserver.h>
#include <functional>
#include <AUI/Util/ADataBinding.h>
#include <AUI/Platform/AWindow.h>

template<typename T, typename Layout>
class AForEachUI: public AViewContainer, public AListModelObserver<T>::IListModelListener {
public:
    using List = _<IListModel<T>>;
    using Factory = std::function<_<AView>(const T& value, size_t index)>;
private:
    _<AListModelObserver<T>> mObserver;

    Factory mFactory;

public:
    AForEachUI(const List& list):
        mObserver(_new<AListModelObserver<T>>(this)) {
        setLayout(_new<Layout>());
        setModel(list);
    }

    void setModel(const List& list) {
        mObserver->setModel(list);
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
        updateLayout();
    }

    void onDataChanged() override {
        updateLayout();
    }

    void operator-(const Factory& f) {
        mFactory = f;
        setModel(mObserver->getModel());
    }
};

#define ui_for(value, model, layout) _new<AForEachUI<std::decay_t<decltype(model)>::stored_t::stored_t, layout>>(model) - [&](const std::decay_t<decltype(model)>::stored_t::stored_t& value, unsigned index) -> _<AView>
