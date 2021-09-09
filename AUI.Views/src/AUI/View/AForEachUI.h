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
    using Factory = std::function<_<AView>(const T&)>;
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
        addView(at, mFactory(value));
    }

    void updateItem(size_t at, const T& value) override {
        // TODO optimize
        removeView(at);
        addView(at, mFactory(value));
    }

    void removeItem(size_t at) override {
        removeView(at);
    }

    void onDataCountChanged() override {
        AWindow::current()->updateLayout();
    }

    void onDataChanged() override {
        AWindow::current()->updateLayout();
    }

    void operator-(const Factory& f) {
        mFactory = f;
        setModel(mObserver->getModel());
    }
};

#define ui_for(key, model, layout) _new<AForEachUI<std::decay_t<decltype(model)>::stored_t::stored_t, layout>>(model) - [](const std::decay_t<decltype(model)>::stored_t::stored_t& key) -> _<AView>