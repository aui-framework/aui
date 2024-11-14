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

#include "AUI/Traits/concepts.h"
#include <functional>
#include <type_traits>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Traits/members.h>


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

template<typename Model, typename View, typename ModelField, typename Getter, aui::invocable<View*, const ModelField&> Setter>
class ADataBindingLinker {
private:
    ADataBinding<Model>* mBinder;
    Setter setter;
    std::decay_t<ModelField>(Model::*mField);
    ASignal<std::decay_t<Getter>>(View::*mGetter);

public:
    ADataBindingLinker(ADataBinding<Model>* binder, ASignal<std::decay_t<Getter>>(View::*getter), Setter setter,
                       std::decay_t<ModelField>(Model::*field)):
            mBinder(binder), mGetter(getter), setter(std::move(setter)), mField(field) {}

    ADataBinding<Model>* getBinder() const {
        return mBinder;
    }

    auto getSetterFunc() const {
        return setter;
    }

    auto getField() const {
        return mField;
    }

    auto getGetter() const {
        return mGetter;
    }
};
template<typename Model, typename Data, typename Projection>
class ADataBindingLinker2 {
private:
    ADataBinding<Model>* mBinder;
    Data(Model::*mField);
    Projection mProjection;

public:
    ADataBindingLinker2(ADataBinding<Model>* binder, Data(Model::*field), Projection projection = nullptr) : mBinder(binder), mField(field), mProjection(std::move(projection)) {}

    ADataBinding<Model>* getBinder() const {
        return mBinder;
    }

    auto getField() const {
        return mField;
    }

    Projection getProjection() const {
        return mProjection;
    }
};

/**
 * @brief Data binding implementation.
 * @tparam Model Your model type.
 * @details
 * <p>
 * If const reference of your model passed, ADataBinding will create and manage its own copy of your model.
 * </p>
 * <p>
 * If pointer of your model passed, ADataBinding will reference to your model and write directly to your model. When
 * ADataBinding is destructed the pointer will not be deleted.
 * </p>
 *
 * <p>Example:</p>
 * @code{cpp}
 * _new<ATextField>() && dataBinding(&User::username)
 * @endcode
 * <p>This code will bind ATextField with username field in the User model.</p>
 *
 * <p>Another example:</p>
 * <img src="https://github.com/aui-framework/aui/raw/develop/docs/imgs/Screenshot_20230705_173329.png">
 * <img src="https://github.com/aui-framework/aui/raw/develop/docs/imgs/Recording_20230705_at_17.51.14.gif">
 * @code{cpp}
 * class MyWindow: public AWindow {
 * public:
 *     MyWindow(): AWindow("Test") {
 *
 *         struct Model {
 *             int value = 0;
 *         };
 *
 *         auto data = _new<ADataBinding<Model>>(Model{});
 *
 *         data->addObserver(&Model::value, [](int v) {
 *             ALogger::info("Debug") << "New value: " << v;
 *         });
 *
 *         setContents(Centered {
 *             Vertical {
 *                 Label { } let {
 *                     data->addObserver(&Model::value, [it](int v) {
 *                         it->setText("{}"_format(v));
 *                     });
 *                 },
 *                 Horizontal{
 *                     Button{"+"}.clicked(this, [data] {
 *                         data->getEditableModel().value += 1;
 *                         data->notifyUpdate();
 *                     }),
 *                     Button{"-"}.clicked(this, [data] {
 *                         data->getEditableModel().value -= 1;
 *                         data->notifyUpdate();
 *                     }),
 *                 },
 *             }
 *         });
 *     }
 * };
 * @endcode
 * <p>Here, we use getEditableModel() in order to change data in our model and notifyUpdate() to notify.</p>
 * <p>Also, we use @ref #let construction to define custom format for label.</p>
 *
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

    /**
     * @brief Create a connection to setter only.
     * @param field pointer-to-member-field of model.
     * @param setterFunc pointer-to-member-function setter.
     */
    template<typename View, typename ModelField, typename SetterArg>
    auto operator()(ModelField(Model::*field), void(View::*setterFunc)(SetterArg)) {
        AUI_ASSERT(setterFunc != nullptr);
        return operator()<View, ModelField>(field, (ASignal<ModelField>(View::*))nullptr, setterFunc);
    }

    /**
     * @brief Create a connection to specified lambda setter only.
     * @param field pointer-to-member-field of model.
     * @param setterLambda lambda which accepts reference to your view type and const reference to data (see examples).
     * @details
     * View type is deduces from the first argument of your lambda.
     * @code{cpp}
     * struct Model {
     *   AString text;
     * };
     * auto model = _new<ADataBinding<Model>>(Model{});
     * _new<ALabel>() && model(&Model::text, [](ALabel& view, const AString& data) {
     *    view.setText(data);
     * });
     * @endcode
     */
    template<typename ModelField, typename SetterLambda>
    auto operator()(ModelField(Model::*field), SetterLambda setterLambda) {
        using lambda_args = aui::lambda_info<SetterLambda>::args;
        return aui::tuple_visitor<lambda_args>::for_each_all([&]<typename ViewReference, typename DataArgument>() {
            static_assert(std::is_reference_v<ViewReference>, "View is expected to be a reference");
            static_assert(aui::convertible_to<ModelField, DataArgument>, "lambda's argument is expected to be constructible from ModelField");
            using View = std::decay_t<ViewReference>;
            return operator()<View, ModelField>(field, (ASignal<ModelField>(View::*))nullptr, [setterLambda = std::move(setterLambda)](View* view, DataArgument dataArgument) {
                std::invoke(setterLambda, *view, std::forward<DataArgument>(dataArgument));
            });
        });
    }

    /**
     * @brief Create a connection to specified pointer-to-member-field signal and pointer-to-member-function setter.
     * @param field pointer-to-member-field of model.
     */
    template<typename View, typename ModelField, typename GetterRV, aui::invocable<View*, const ModelField&> Setter>
    auto operator()(ModelField(Model::*field),
                    ASignal<GetterRV>(View::*getter),
                    Setter setter = (void(View::*)(const ModelField&))nullptr) {
        return ADataBindingLinker<Model, View, std::decay_t<ModelField>, GetterRV, Setter>(this, getter, std::move(setter), field);
    }

    /**
     * @brief Create a connection via ADataBindingDefault.
     * @param field pointer-to-member-field of model.
     * @details
     * ADataBindingDefault must be defined for model type and your view.
     */
    template<typename Data>
    auto operator()(Data(Model::*field)) {
        return ADataBindingLinker2<Model, Data, std::nullptr_t>(this, field, nullptr);
    }

    /**
     * @brief Create a connection via ADataBindingDefault and projection (setter only).
     * @details
     * ADataBindingDefault must be defined for model type and your view.
     */
    template<typename Data, aui::invocable<Data> Projection>
    ADataBindingLinker2<Model, Data, Projection> operator()(Data(Model::*field), Projection projection) {
        return ADataBindingLinker2<Model, Data, Projection>(this, field, std::move(projection));
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
    void notifyUpdate(ModelField(Model::*field)) {
        union converter {
            unsigned i;
            decltype(field) p;
        } c;
        c.p = field;
        notifyUpdate(nullptr, c.i);
    }

    template<typename ModelField, aui::convertible_to<ModelField> U>
    void setValue(ModelField(Model::*field), U&& value) {
        mModel->*field = std::move(value);
        notifyUpdate(field);
    }

    void addObserver(Observer applier) {
        mLinkObservers << std::move(applier);
        if (mModel) {
            mLinkObservers.last()(*mModel, -1);
        }
    }

    template<aui::invocable T>
    void addObserver(T&& applier) {
        addObserver([applier = std::forward<T>(applier)](const Model&, unsigned) {
            applier();
        });
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

template<typename Klass1, typename View, typename Model, typename ModelField, typename GetterRV, typename SetterArg>
_<Klass1> operator&&(const _<Klass1>& modelBinding, const ADataBindingLinker<Model, View, ModelField, GetterRV, SetterArg>& linker) {
    union converter {
        unsigned i;
        decltype(linker.getField()) p;
    };
    if (linker.getGetter()) {
        AObject::connect(modelBinding.get()->*(linker.getGetter()), linker.getBinder(), [modelBinding, linker](const GetterRV& data) {
            AUI_ASSERTX(&linker.getBinder()->getEditableModel(), "please setModel for ADataBinding");
            modelBinding->setSignalsEnabled(false);
            linker.getBinder()->getEditableModel().*(linker.getField()) = data;
            converter c;
            c.p = linker.getField();
            linker.getBinder()->notifyUpdate(modelBinding.get(), c.i);
            modelBinding->setSignalsEnabled(true);
        });
    }

    if constexpr (aui::convertible_to<decltype(linker.getSetterFunc()), bool>) {
        if (!bool(linker.getSetterFunc())) {
            return modelBinding;
        }
    }
    linker.getBinder()->addObserver([modelBinding, linker](const Model& model, unsigned field) {
        converter c;
        c.p = linker.getField();
        if (c.i == field || field == -1) {
            if (modelBinding.get() != linker.getBinder()->getExclusion()) {
                std::invoke(linker.getSetterFunc(), modelBinding.get(), model.*(linker.getField()));
            }
        }
    });

    return modelBinding;
}


namespace aui::detail {
    template<typename ForcedClazz, typename Type>
    struct pointer_to_member {
        template<typename... Args>
        static Type(ForcedClazz::*with_args(std::tuple<Args...>))(Args...) {
            return nullptr;
        }
    };
}

template<typename View, typename Model, typename Data, typename Projection>
_<View> operator&&(const _<View>& object, const ADataBindingLinker2<Model, Data, Projection>& linker) {
    ADataBindingDefault<View, Data>::setup(object);

    constexpr bool is_default_projection = std::is_same_v<Projection, std::nullptr_t>;
    using projection_deduced = std::conditional_t<is_default_projection, std::identity, Projection>;
    static_assert(std::is_invocable_v<projection_deduced, Data>, "projection is expected to accept Data value from model");
    using data_deduced = std::decay_t<std::invoke_result_t<projection_deduced, Data>>;

    using getter = ASignal<Data>(View::*);
    using setter = aui::member<decltype(ADataBindingDefault<View, data_deduced>::getSetter())>;

    using setter_ret = typename setter::return_t;
    using setter_args = typename setter::args;

    using my_pointer_to_member = typename aui::detail::pointer_to_member<View, setter_ret>;

    using pointer_to_setter = decltype(my_pointer_to_member::with_args(std::declval<setter_args>()));

    if constexpr (is_default_projection) {
        getter g = nullptr;
        // getter is optional.
        if constexpr (requires { ADataBindingDefault<View, Data>::getGetter(); }) {
            g = (getter) (ADataBindingDefault<View, Data>::getGetter());
        }
        auto s = static_cast<pointer_to_setter>(ADataBindingDefault<View, Data>::getSetter());

        AUI_ASSERTX(g != nullptr || s != nullptr, "ADataBindingDefault is not defined for View, Data");

        object && (*linker.getBinder())(linker.getField(), g, s);
    } else {
        object && (*linker.getBinder())(linker.getField(), [projection = linker.getProjection()](View& v, const Data& d) {
            auto s = static_cast<pointer_to_setter>(ADataBindingDefault<View, data_deduced>::getSetter());
            std::invoke(s, &v, projection(d));
        });
    }
    return object;
}
