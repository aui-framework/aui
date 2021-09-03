#pragma once


#include "AViewContainer.h"
#include <AUI/Model/IListModel.h>
#include <AUI/Model/AListModelObserver.h>
#include <functional>
#include <AUI/Util/ADataBinding.h>

template<typename T>
class AForEach: public AViewContainer, public AListModelObserver<T>::IListModelListener {
public:
    using List = _<IListModel<T>>;
    using Factory = std::function<_<AView>(_<ADataBinding<T>>&)>;
private:
    _<AListModelObserver<T>> mObserver;

    /**
     * Holds the binding object for the views.
     * @note Should be kept in sync with <code>mViews</code>.
     */
    AVector<_<ADataBinding<T>>> mBindings;

    Factory mFactory;

public:
    AForEach(const List& list, const Factory& factory):
        mObserver(_new<AListModelObserver<T>>(this)),
        mFactory(factory) {
        setModel(list);
        setLayout(_new<AVerticalLayout>());
    }

    void setModel(const List& list) {
        mObserver->setModel(list);
    }

    void insertItem(size_t at, const T& value) override {
        auto binding = _new<ADataBinding<T>>(value);
        mBindings.insert(mBindings.begin() + at, binding);
        addView(at, mFactory(binding));
    }

    void updateItem(size_t at, const T& value) override {
        mBindings[at]->setModel(value);
    }

    void removeItem(size_t at) override {
        mBindings.removeAt(at);
        removeView(at);
    }

    void onDataCountChanged() override {
        AWindow::current()->updateLayout();
    }

    void onDataChanged() override {
        AWindow::current()->updateLayout();
    }
};

