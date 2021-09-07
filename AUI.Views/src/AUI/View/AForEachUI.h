#pragma once


#include "AViewContainer.h"
#include <AUI/Model/IListModel.h>
#include <AUI/Model/AListModelObserver.h>
#include <functional>
#include <AUI/Util/ADataBinding.h>

template<typename T>
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
        setModel(list);
        setLayout(_new<AVerticalLayout>());
    }

    void setModel(const List& list) {
        mObserver->setModel(list);
    }

    void insertItem(size_t at, const T& value) override {
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

    void operator+(const Factory& f) {
        mFactory = f;
    }
};

#define ui_for(key, model) _new<AForEachUI<decltype(model)::stored_t::stored_t>>(model) + [](const decltype(model)::stored_t::stored_t& key) -> _<AView>