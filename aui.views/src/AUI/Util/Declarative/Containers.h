/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Util/Declarative/Concepts.h>
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/View/ALabel.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Layout/AAbsoluteLayout.h>

namespace aui::ui_building {

namespace detail {
template <typename ViewFactory>
struct view_helper {
public:
    view_helper() {}

    operator View() const { return asViewFactory()->operator()(); }
    operator ViewContainer() const { return asViewFactory()->operator()(); }
    auto operator<<(const AString& assEntry) const { return asViewFactory()->operator()() << assEntry; }
    template <typename T>
    auto operator^(const T& t) const {
        return asViewFactory()->operator()() ^ t;
    }
    template <typename T>
    auto operator+(const T& t) const {
        return asViewFactory()->operator()() + t;
    }

    template <typename T>
    auto operator&(const T& t) const {
        return asViewFactory()->operator()() & t;
    }

    template <typename T>
    auto operator|(const T& t) const {
        return asViewFactory()->operator()() | t;
    }

    template <typename T>
    auto operator^(T&& t) const {
        return asViewFactory()->operator()() ^ std::forward<T>(t);
    }
    template <typename T>
    auto operator&(T&& t) const {
        return asViewFactory()->operator()() & std::forward<T>(t);
    }
    template <typename T>
    auto operator&&(T&& t) const {
        return asViewFactory()->operator()() && std::forward<T>(t);
    }
    template <typename T>
    auto operator+(T&& t) const {
        return asViewFactory()->operator()() + std::forward<T>(t);
    }

    auto operator->() const { return asViewFactory()->operator()(); }

    template <typename SignalField, typename Object, typename Function>
    auto connect(SignalField&& signalField, Object&& object, Function&& function) {
        return asViewFactory()->operator()().connect(
            std::forward<SignalField>(signalField), std::forward<Object>(object), std::forward<Function>(function));
    }

    template <typename SignalField, typename Function>
    auto connect(SignalField&& signalField, Function&& function) {
        return asViewFactory()->operator()().connect(
            std::forward<SignalField>(signalField), std::forward<Function>(function));
    }

private:
    [[nodiscard]] ViewFactory* asViewFactory() const {
        return const_cast<ViewFactory*>(static_cast<const ViewFactory*>(this));
    }
};


template <typename Layout, aui::derived_from<AViewContainer> Container = AViewContainer>
struct layouted_container_factory_impl {
private:
    AVector<View> mViews;

public:
    template <typename... Views>
    layouted_container_factory_impl(Views&&... views) {
        mViews.reserve(sizeof...(views));
        static_assert(
            (LayoutItemAny<Views> && ...),
            "One of the items passed to declarative container is not valid. "
            "Please check your compiler's diagnostics on constraint satisfaction.");
        aui::parameter_pack::for_each(
            [this]<LayoutItemAny Item>(Item&& item) {
                constexpr bool isView = LayoutItemView<Item>;
                constexpr bool isViewGroup = LayoutItemViewGroup<Item>;
                constexpr bool isInvokable = LayoutItemViewFactory<Item>;

                if constexpr (isViewGroup) {
                    auto asViewGroup = ViewGroup(item);
                    mViews << std::move(asViewGroup);
                } else if constexpr (isView) {
                    auto asView = View(item);
                    mViews << std::move(asView);
                } else if constexpr (isInvokable) {
                    mViews << item();
                }
            },
            std::forward<Views>(views)...);
    }

    _<Container> operator()() {
        auto c = _new<Container>();
        if constexpr (!std::is_same_v<Layout, std::nullopt_t>) {
            c->setLayout(std::make_unique<Layout>());
        }
        c->setViews(std::move(mViews));
        return c;
    }
};

template <typename Layout, aui::derived_from<AViewContainer> Container = AViewContainer>
struct layouted_container_factory_impl_with_expanding : layouted_container_factory_impl<Layout, Container> {
public:
    using layouted_container_factory_impl<Layout, Container>::layouted_container_factory_impl;

    struct Expanding : view_helper<Expanding>, layouted_container_factory_impl<Layout, Container> {
    public:
        template <typename... Views>
        Expanding(Views&&... views) : layouted_container_factory_impl<Layout, Container>(std::forward<Views>(views)...) {}

        _<Container> operator()() {
            return layouted_container_factory_impl<Layout, Container>::operator()() AUI_LET { it->setExpanding(); };
        }
    };
};

template <typename Layout, aui::derived_from<AViewContainer> Container = AViewContainer>
struct layouted_container_factory
  : view_helper<layouted_container_factory<Layout, Container>>,
    layouted_container_factory_impl_with_expanding<Layout, Container> {
    template <typename... Views>
    layouted_container_factory(Views&&... views)
      : layouted_container_factory_impl_with_expanding<Layout, Container>(std::forward<Views>(views)...) {}
};

}   // namespace detail

/**
 * @brief Declarative view trait.
 */
template <typename View>
struct view : detail::view_helper<view<View>> {
public:
    template <typename... Args>
    view(Args&&... args) : mView(_new<View>(std::forward<Args>(args)...)) {}

    _<View>& operator()() { return mView; }

    operator _<View>&() { return mView; }

private:
    _<View> mView;
};

static_assert(std::is_convertible_v<view<AView>, View>,
              "====================> AUI: declarative view wrapper (declarative::view) must be convertible to _<AView>");


/**
 * @brief Declarative view container trait.
 */
template <typename Layout, aui::derived_from<AViewContainer> Container = AViewContainer>
using view_container_layout = detail::layouted_container_factory<Layout, Container>;

}   // namespace aui::ui_building

namespace declarative {

/**
 * @brief Extra styles wrapper.
 * @ingroup views
 * @details
 * ```cpp
 * Label { "Default label" },
 * Style{
 *     {
 *         c(".btn"),
 *         BackgroundSolid { 0xff0000_rgb },
 *     },
 * } ({
 *     Label { "Red label" },
 *     Label { "Another red label" },
 * }),
 * ```
 */
struct Style {
public:
    Style(std::initializer_list<Rule> rules) : mStylesheet(_new<AStylesheet>(AStylesheet(rules))) {}

    Style& operator()(AVector<_<AView>> views) {
        for (const auto& view : views) {
            AUI_ASSERTX(view->extraStylesheet() == nullptr, "extra stylesheet already specified");
            view->setExtraStylesheet(mStylesheet);
        }
        mViews = std::move(views);
        return *this;
    }

    operator AVector<_<AView>>() noexcept { return std::move(mViews); }

private:
    _<AStylesheet> mStylesheet;
    AVector<_<AView>> mViews;
};
}   // namespace declarative


#define AUI_DETAIL_BINARY_OP(op)                                                          \
    template <typename Rhs>                                                               \
    auto operator op(aui::ui_building::LayoutItemViewFactory auto&& factory, Rhs&& rhs) { \
        return factory() op std::forward<Rhs>(rhs);                                       \
    }

AUI_DETAIL_BINARY_OP(&) // forwards AUI_OVERRIDE_STYLE
AUI_DETAIL_BINARY_OP(^) // forwards let
AUI_DETAIL_BINARY_OP(<<) // forwards stylesheet name assignment


namespace declarative {
template <typename Layout, typename... Args>
inline auto _container(aui::ui_building::ViewGroup views, Args&&... args) {
    auto c = _new<AViewContainer>();
    c->setLayout(std::make_unique<Layout>(std::forward<Args>(args)...));

    c->setViews(std::move(views));

    return c;
}

inline auto _form(const AVector<std::pair<std::variant<AString, _<AView>>, _<AView>>>& views) {
    auto c = _new<AViewContainer>();
    c->setLayout(std::make_unique<AAdvancedGridLayout>(2, int(views.size())));
    c->setExpanding({ 2, 0 });
    for (const auto& v : views) {
        try {
            c->addView(_new<ALabel>(std::get<AString>(v.first)));
        } catch (const std::bad_variant_access&) {
            c->addView(std::get<_<AView>>(v.first));
        }
        v.second->setExpanding({ 2, 0 });
        c->addView(v.second);
    }

    return c;
}

/**
 * @brief Places views in a column.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/vertical.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AVerticalLayout</dt>
 *  </dl>
 * </p>
 */
using Vertical = aui::ui_building::view_container_layout<AVerticalLayout>;

/**
 * @brief Places views in a row.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/horizontal.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AHorizontalLayout</dt>
 *  </dl>
 * </p>
 */
using Horizontal = aui::ui_building::view_container_layout<AHorizontalLayout>;

/**
 * @brief Places views in a stack, centering them.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/stacked2.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AStackedLayout</dt>
 *  </dl>
 * </p>
 */
using Stacked = aui::ui_building::view_container_layout<AStackedLayout>;

/**
 * @brief Places views according to specified xy coordinates.
 * <p>
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> AAbsoluteLayout</dt>
 *  </dl>
 * </p>
 */
using Absolute = aui::ui_building::view_container_layout<AAbsoluteLayout>;

/**
 * Does not actually set the layout. The views' geometry is determined manually.
 * @deprecated Use AAbsoluteLayout instead.
 * <p>
 *  <dl>
 *    <dt><b>View:</b> AViewContainer</dt>
 *    <dt><b>Layout manager:</b> null</dt>
 *  </dl>
 * </p>
 */
using CustomLayout = aui::ui_building::view_container_layout<std::nullopt_t>;

/**
 * <p>
 * <code>Center</code> is an alias to Stacked. When Stacked is used only for centering views, you can use
 * this alias in order to improve understanding of your code.
 * </p>
 */
using Centered = Stacked;
}