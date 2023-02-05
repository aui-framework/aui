// AUI Framework - Declarative UI toolkit for modern C++20
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

#include <type_traits>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Traits/members.h>
#include <AUI/Util/AFieldObservable.h>


template<typename View, typename FieldType>
struct ADataBindingDefault {
public:
    /**
     * Called then view linked with field.
     * @param view
     */
    static void setup(const _<View>& view) {}

    static ASignal<FieldType>(View::*getGetter()) {
        return nullptr;
    }
    static void(View::*getSetter())(const FieldType& v) {
        return nullptr;
    }
};

template <typename Model>
class ADataBinding;

template<typename Model, typename Klass, typename ModelField, typename Getter, typename Setter>
class ADataBindingLinker {
private:
    ADataBinding<Model>* mBinder;
    void(Klass::*mSetterFunc)(Setter);
    std::decay_t<ModelField>(Model::*mField);
    ASignal<std::decay_t<Getter>>(Klass::*mGetter);

public:
    ADataBindingLinker(ADataBinding<Model>* binder, ASignal<std::decay_t<Getter>>(Klass::*getter), void (Klass::*setterFunc)(Setter),
                       std::decay_t<ModelField>(Model::*field)):
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

/**
 * Data binding implementation.
 * <p>
 * If const reference of your model passed, ADataBinding will create and manage its own copy of your model.
 * </p>
 * <p>
 * If pointer of your model passed, ADataBinding will reference to your model and write directly to your model. When
 * ADataBinding is destructed the pointer will not be deleted.
 * </p>
 *
 * <p>Example:</p>
 * <code>
 * _new<ATextField>() && dataBinding(&User::username)
 * </code>
 * <p>This code will bind ATextField with username field in the User model.</p>
 *
 * @tparam Model Your model type.
 */
template <typename Model>
class ADataBinding: public AObject {
private:
    using Observer = std::function<void(const Model& model, unsigned)>;
    ADeque<Observer> mLinkObservers;

    Model* mModel = nullptr;
    bool mOwning = false;

    void* mExcept = nullptr;

public:

    ADataBinding() = default;
    explicit ADataBinding(const Model& m)
    {
        setModel(m);
    }
    explicit ADataBinding(Model* m)
    {
        setModel(m);
    }

    virtual ~ADataBinding() {
        if (mOwning) {
            delete mModel;
        }
    }
    template<typename View, typename ModelField, typename SetterArg>
    ADataBindingLinker<Model, View, std::decay_t<ModelField>, std::decay_t<ModelField>, SetterArg> operator()(ModelField(Model::*field), void(View::*setterFunc)(SetterArg)) {
        assert(setterFunc != nullptr);
        return ADataBindingLinker<Model, View, std::decay_t<ModelField>, std::decay_t<ModelField>, SetterArg>(this, nullptr, setterFunc, field);
    }
    template<typename View, typename ModelField, typename GetterRV, typename SetterArg>
    ADataBindingLinker<Model, View, std::decay_t<ModelField>, GetterRV, SetterArg> operator()(ModelField(Model::*field),
                                                                                              ASignal<GetterRV>(View::*getter),
                                                                                              void(View::*setterFunc)(SetterArg) = nullptr) {
        assert((getter != nullptr || setterFunc != nullptr) &&
               "implement ADataBindingDefault for your view in order to use default binding");

        return ADataBindingLinker<Model, View, std::decay_t<ModelField>, GetterRV, SetterArg>(this, getter, setterFunc, field);
    }

    template<typename Data>
    ADataBindingLinker2<Model, Data> operator()(Data(Model::*field)) {
        return ADataBindingLinker2<Model, Data>(this, field);
    }
    const Model& getModel() const noexcept {
        return *mModel;
    }

    Model const * operator->() const noexcept {
        return &getModel();
    }

    Model& getEditableModel() {
        return *mModel;
    }
    void setModel(const Model& model) {
        if (mOwning) {
            delete mModel;
        }
        mOwning = true;
        mModel = new Model(model);
        notifyUpdate();
    }

    void setModel(Model* model) {
        if (mOwning) {
            delete mModel;
        }
        mOwning = false;
        mModel = model;
        notifyUpdate();
    }

    const void* getExclusion() const {
        return mExcept;
    }

    void notifyUpdate(void* except = nullptr, unsigned field = -1) {
        mExcept = except;
        for (auto& applier : mLinkObservers) {
            applier(*mModel, field);
        }
        emit modelChanged;
    }

    template<typename ModelField>
    void setValue(ModelField(Model::*field), ModelField value) {
        mModel->*field = std::move(value);
        union converter {
            unsigned i;
            decltype(field) p;
        } c;
        c.p = field;
        notifyUpdate(nullptr, c.i);
    }

    void addObserver(const Observer& applier) {
        mLinkObservers << applier;
        if (mModel) {
            mLinkObservers.last()(*mModel, -1);
        }
    }

    template<typename ModelField, typename FieldObserver>
    void addObserver(ModelField(Model::*field), FieldObserver&& observer) {
        mLinkObservers << [observer = std::forward<FieldObserver>(observer), field] (const Model& model, unsigned index) {
            union converter {
                unsigned i;
                decltype(field) p;
            } c;
            c.p = field;
            if (c.i == index || index == -1) {
                observer(model.*field);
            }
        };
        if (mModel) {
            mLinkObservers.last()(*mModel, -1);
        }
    }


signals:
    /**
     * @brief Data in the model has changed.
     */
    emits<> modelChanged;
};

template<typename Klass1, typename Klass2, typename Model, typename ModelField, typename GetterRV, typename SetterArg>
_<Klass1> operator&&(const _<Klass1>& object, const ADataBindingLinker<Model, Klass2, ModelField, GetterRV, SetterArg>& linker) {
    union converter {
        unsigned i;
        decltype(linker.getField()) p;
    };
    if (linker.getGetter()) {
        AObject::connect(object.get()->*(linker.getGetter()), linker.getBinder(), [object, linker](const GetterRV& data) {
            assert(("please setModel for ADataBinding" && &linker.getBinder()->getEditableModel()));
            object->setSignalsEnabled(false);
            linker.getBinder()->getEditableModel().*(linker.getField()) = data;
            converter c;
            c.p = linker.getField();
            linker.getBinder()->notifyUpdate(object.get(), c.i);
            object->setSignalsEnabled(true);
        });
    }

    if (linker.getSetterFunc()) {
        linker.getBinder()->addObserver([object, linker](const Model& model, unsigned field) {
            converter c;
            c.p = linker.getField();
            if (c.i == field || field == -1) {
                if (object.get() != linker.getBinder()->getExclusion()) {
                    (object.get()->*(linker.getSetterFunc()))(model.*(linker.getField()));
                }
            }
        });
    }

    return object;
}


namespace aui::detail {
    template<typename ForcedClazz, typename Type>
    struct pointer_to_member {
        template<typename... Args>
        static Type(ForcedClazz::*with_args(aui::type_list<Args...>))(Args...) {
            return nullptr;
        }
    };
}

template<typename View, typename Model, typename Data>
_<View> operator&&(const _<View>& object, const ADataBindingLinker2<Model, Data>& linker) {
    ADataBindingDefault<View, Data>::setup(object);

    using setter = aui::member<decltype(ADataBindingDefault<View, Data>::getSetter())>;

    using setter_ret = typename setter::type;
    using setter_args = typename setter::args;

    using my_pointer_to_member = typename aui::detail::pointer_to_member<View, setter_ret>;

    using pointer_to_setter = decltype( my_pointer_to_member::with_args(std::declval<setter_args>()));

    object && (*linker.getBinder())(linker.getField(),
                                    static_cast<ASignal<Data>(View::*)>(ADataBindingDefault<View, Data>::getGetter()),
                                    static_cast<pointer_to_setter>(ADataBindingDefault<View, Data>::getSetter()));
    return object;
}

template<typename View, typename Data>
_<View> operator&&(const _<View>& object, AFieldObservable<Data>& observable) {
    typename std::decay_t<decltype(observable)>::ObserverHandle observerHandle = nullptr;
    if (ADataBindingDefault<View, Data>::getSetter()) {
        observerHandle = observable << [object = object.get()](Data newValue) {
            (object->*ADataBindingDefault<View, Data>::getSetter())(std::move(newValue));
        };
    }
    if (auto getter = ADataBindingDefault<View, Data>::getGetter()) {
        AObject::connect(object.get()->*getter, object, [&observable, observerHandle](Data newValue) {
            observable.setValue(std::move(newValue), observerHandle);
        });
    }

    return object;
}
