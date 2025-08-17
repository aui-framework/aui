#pragma once

#include <AUI/Common/detail/property.h>

/**
 * @brief Property implementation to use with custom getter/setter.
 * @ingroup property_system
 * @details
 * <!-- aui:experimental -->
 * You can use this way if you are required to define custom behaviour on getter/setter. As a downside, you have to
 * write extra boilerplate code: define property, data field, signal, getter and setter checking equality. Also,
 * APropertyDef requires the class to derive `AObject`. Most of AView's properties are defined this way.
 *
 * See [property system](property_system) for usage examples.
 *
 * ## Performance considerations
 *
 * APropertyDef [does not involve](https://godbolt.org/z/cYTrc3PPf ) extra runtime overhead between assignment and
 * getter/setter.
 */
template <
    typename M, aui::invocable<M&> Getter, aui::invocable<M&, std::invoke_result_t<Getter, M&>> Setter,
typename SignalArg>
struct APropertyDef {
    /**
     * @brief AObject which this property belongs to.
     */
    const M* base;
    using Model = M;

    /**
     * @brief Getter. Can be pointer-to-member(function or field) or lambda.
     */
    Getter get;

    /**
     * @brief Setter. Can be pointer-to-member(function or field) or lambda.
     * @details
     * The setter implementation typically emits `changed` signal. If it is, it must emit changes only if value is
     * actually changed.
     * ```cpp
     * void setValue(int value) {
     *   if (mValue == value) {
     *     return;
     *   }
     *   mValue = value;
     *   emit mValueChanged(valueChanged);
     * }
     * ```
     */
    Setter set;
    using GetterReturnT = decltype(std::invoke(get, base));
    using Underlying = std::decay_t<GetterReturnT>;

    /**
     * @brief Reference to underlying signal emitting on value changes.
     */
    const emits<SignalArg>& changed;
    static_assert(aui::same_as<Underlying , std::decay_t<SignalArg>>, "different getter result and signal arg?");

    // this ctor effectively prohibits designated initialization, i.e., this one is not possible:
    //
    // auto size() const {
    //     return APropertyDef {
    //         .base = this,
    //         .get = &AView::mSize,
    //         .set = &AView::setSize,
    //         .changed = mSizeChanged,
    //     };
    // }
    //
    // deduction in designated initializers is relatively recent feature.
    APropertyDef(const M* base, Getter get, Setter set, const emits<SignalArg>& changed)
        : base(base), get(std::move(get)), set(std::move(set)), changed(changed) {}

    template <aui::convertible_to<Underlying> U>
    APropertyDef& operator=(U&& u) {
        std::invoke(set, *const_cast<Model*>(base), std::forward<U>(u));
        return *this;
    }

    [[nodiscard]]
    GetterReturnT value() const noexcept {
        return std::invoke(get, base);
    }

    [[nodiscard]]
    GetterReturnT operator*() const noexcept {
        return std::invoke(get, base);
    }

    [[nodiscard]]
    const Underlying* operator->() const noexcept {
        return &std::invoke(get, base);
    }

    [[nodiscard]] operator GetterReturnT() const noexcept { return std::invoke(get, base); }

    [[nodiscard]]
    M* boundObject() const {
        return const_cast<M*>(base);
    }

    /**
     * @brief Makes a readonly [projection](UIDataBindingTest_Label_via_declarative_projection) of this property.
     */
    template <aui::invocable<const Underlying&> Projection>
    [[nodiscard]]
    auto readProjected(Projection&& projection) noexcept {
        return aui::detail::property::makeReadonlyProjection(std::move(*this), std::forward<Projection>(projection));
    }

    /**
     * @brief Makes a bidirectional [projection](UIDataBindingTest_Label_via_declarative_projection) of this property.
     */
    template <
        aui::invocable<const Underlying&> ProjectionRead,
        aui::invocable<const std::invoke_result_t<ProjectionRead, Underlying>&> ProjectionWrite>
    [[nodiscard]]
    auto biProjected(ProjectionRead&& projectionRead, ProjectionWrite&& projectionWrite) noexcept {
        return aui::detail::property::makeBidirectionalProjection(
            std::move(*this), std::forward<ProjectionRead>(projectionRead),
            std::forward<ProjectionWrite>(projectionWrite));
    }

    /**
     * @brief Makes a bidirectional projection of this property (by a single aui::lambda_overloaded).
     */
    template <aui::detail::property::ProjectionBidirectional<Underlying> Projection>
    [[nodiscard]]
    auto biProjected(Projection&& projectionBidirectional) noexcept {
        return aui::detail::property::makeBidirectionalProjection(std::move(*this), projectionBidirectional);
    };

    /**
     * @return @copybrief aui::PropertyModifier See aui::PropertyModifier.
     */
    aui::PropertyModifier<APropertyDef> writeScope() noexcept {
        return { std::move(*this) };
    }

    /**
     * @brief Notify observers that a change was occurred (no preconditions).
     */
    void notify() {
        if (changed.hasOutgoingConnections()) {
            emit changed(this->value());
        }
    }

private:
    friend class AObject;
    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(std::move(*this));
    }
};

// implementation of property modifier for APropertyDef (in comparison to AProperty) has to call getter, store a copy
// within PropertyModifier and call setter upon destruction.
// this behaviour described here - https://aui-framework.github.io/develop/structAPropertyDef.html#declaration
template <
    typename M, aui::invocable<M&> Getter, aui::invocable<M&, std::invoke_result_t<Getter, M&>> Setter,
    typename SignalArg>
class aui::PropertyModifier<APropertyDef<M, Getter, Setter, SignalArg>> {
public:
    using Property = APropertyDef<M, Getter, Setter, SignalArg>;
    using Underlying = std::decay_t<typename Property::Underlying>;
    PropertyModifier(Property owner): mOwner(std::move(owner)), mCopyOfValue(*mOwner) {}
    ~PropertyModifier() {
        mOwner = std::move(mCopyOfValue); // calls setter
    }

    [[nodiscard]]
    Underlying& value() const noexcept {
        return mCopyOfValue;
    }

    [[nodiscard]]
    Underlying* operator->() const noexcept {
        return &value();
    }

private:
    Property mOwner;
    mutable Underlying mCopyOfValue;
};
