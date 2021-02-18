/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <type_traits>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Common/ASignal.h>


template<typename View, typename FieldType>
struct ADataBindingDefault {
public:
    static ASignal<FieldType>(View::*getGetter()) {
        return nullptr;
    }
    static void(View::*getSetter())(const FieldType& v) {
        return nullptr;
    }
};

template <typename Model>
class ADataBinding;

template<typename Model, typename Klass, typename Data>
class ADataBindingLinker {
private:
    ADataBinding<Model>* mBinder;
    void(Klass::*mSetterFunc)(Data);
    std::decay_t<Data>(Model::*mField);
    ASignal<std::decay_t<Data>>(Klass::*mGetter);

public:
    ADataBindingLinker(ADataBinding<Model>* binder, ASignal<std::decay_t<Data>>(Klass::*getter), void (Klass::*setterFunc)(Data),
                       std::decay_t<Data>(Model::*field)):
        mBinder(binder), mGetter(getter), mSetterFunc(setterFunc), mField(field) {}

    ADataBinding<Model>* getBinder() const {
        return mBinder;
    }

    auto getSetterFunc() const {
        return mSetterFunc;
    }

    auto getField() const {
        return mField;
    }

    auto getGetter() const {
        return mGetter;
    }
};
template<typename Model, typename Data>
class ADataBindingLinker2 {
private:
    ADataBinding<Model>* mBinder;
    Data(Model::*mField);

public:
    ADataBindingLinker2(ADataBinding<Model>* binder, Data(Model::*field)) : mBinder(binder), mField(field) {}

    ADataBinding<Model>* getBinder() const {
        return mBinder;
    }

    auto getField() const {
        return mField;
    }
};

template <typename Model>
class ADataBinding: public AObject {
private:
    using Applier = std::function<void(const Model& model)>;
    ADeque<Applier> mLinkAppliers;

    Model mModel;

    void* mExcept = nullptr;

public:

    virtual ~ADataBinding() = default;
    template<typename Klass, typename Data>
    ADataBindingLinker<Model, Klass, Data> operator()(std::decay_t<Data>(Model::*field), void(Klass::*setterFunc)(Data)) {
        assert(setterFunc != nullptr);
        return ADataBindingLinker<Model, Klass, Data>(this, nullptr, setterFunc, field);
    }
    template<typename Klass, typename Data1, typename Data2>
    ADataBindingLinker<Model, Klass, Data1> operator()(std::decay_t<Data1>(Model::*field), ASignal<Data2>(Klass::*getter), void(Klass::*setterFunc)(Data1) = nullptr) {
        static_assert(std::is_same_v<std::decay_t<Data2>, std::decay_t<Data1>>, "ASignal argument, field and setter"
                                                                                "argument types must be the same");
        assert((getter != nullptr || setterFunc != nullptr) &&
               "implement ADataBindingDefault for your type in order to use default binding");
        assert(getter != nullptr);
        return ADataBindingLinker<Model, Klass, Data1>(this, getter, setterFunc, field);
    }

    template<typename Data>
    ADataBindingLinker2<Model, Data> operator()(Data(Model::*field)) {
        return ADataBindingLinker2<Model, Data>(this, field);
    }
    const Model& getModel() const {
        return mModel;
    }
    Model& getEditableModel() {
        return mModel;
    }
    void setModel(const Model& model) {
        mModel = model;
        update();
    }

    const void* getExclusion() const {
        return mExcept;
    }

    void update(void* except = nullptr) {
        mExcept = except;
        for (auto& applier : mLinkAppliers) {
            applier(mModel);
        }
    }

    void addApplier(const Applier& applier) {
        mLinkAppliers << applier;
        mLinkAppliers.last()(mModel);
    }
};

template<typename Klass1, typename Klass2, typename Model, typename Data>
_<Klass1> operator&&(const _<Klass1>& object, const ADataBindingLinker<Model, Klass2, Data>& linker) {
    if (linker.getGetter()) {
        AObject::connect(object.get()->*(linker.getGetter()), linker.getBinder(), [object, linker](const Data& data) {
            object->setSignalsEnabled(false);
            linker.getBinder()->getEditableModel().*(linker.getField()) = data;
            linker.getBinder()->update(object.get());
            object->setSignalsEnabled(true);
        });
    }

    if (linker.getSetterFunc()) {
        linker.getBinder()->addApplier([object, linker](const Model& model) {
            if (object.get() != linker.getBinder()->getExclusion()) {
                (object.get()->*(linker.getSetterFunc()))(model.*(linker.getField()));
            }
        });
    }

    return object;
}

template<typename View, typename Model, typename Data>
_<View> operator&&(const _<View>& object, const ADataBindingLinker2<Model, Data>& linker) {
    object && (*linker.getBinder())(linker.getField(),
                                    ADataBindingDefault<View, Data>::getGetter(),
                                    ADataBindingDefault<View, Data>::getSetter());
    return object;
}
