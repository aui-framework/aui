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

#include <AUI/View/AViewContainer.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/callables.h>
#include <AUI/Traits/parameter_pack.h>
#include <AUI/ASS/ASS.h>


namespace aui::ui_building {

    using View = _<AView>;
    using ViewContainer = _<AViewContainer>;
    using ViewGroup = AVector<_<AView>>;
    using ViewOrViewGroup = std::variant<_<AView>, AVector<_<AView>>>;

    template<typename ViewFactory>
    struct view_helper {
    public:
        view_helper() {}

        operator View() const {
            return asViewFactory()->operator()();
        }
        operator ViewContainer() const {
            return asViewFactory()->operator()();
        }
        auto operator<<(const AString& assEntry) const {
            return asViewFactory()->operator()() << assEntry;
        }
        template<typename T>
        auto operator^(const T& t) const {
            return asViewFactory()->operator()() ^ t;
        }
        template<typename T>
        auto operator+(const T& t) const {
            return asViewFactory()->operator()() + t;
        }

        template<typename T>
        auto operator^(T&& t) const {
            return asViewFactory()->operator()() ^ std::forward<T>(t);
        }
        template<typename T>
        auto operator&&(T&& t) const {
            return asViewFactory()->operator()() && std::forward<T>(t);
        }
        template<typename T>
        auto operator+(T&& t) const {
            return asViewFactory()->operator()() + std::forward<T>(t);
        }

        auto operator->() const {
            return asViewFactory()->operator()();
        }


        template<typename SignalField, typename Object, typename Function>
        auto connect(SignalField&& signalField, Object&& object, Function&& function) {
            return asViewFactory()->operator()().connect(std::forward<SignalField>(signalField), std::forward<Object>(object), std::forward<Function>(function));
        }

        template<typename Object, typename Function>
        auto clicked(Object&& object, Function&& function) {
            return connect(&AView::clicked, std::forward<Object>(object), std::forward<Function>(function));
        }

        template<typename SignalField, typename Function>
        auto connect(SignalField&& signalField, Function&& function) {
            return asViewFactory()->operator()().connect(std::forward<SignalField>(signalField), std::forward<Function>(function));
        }
    private:
        [[nodiscard]]
        ViewFactory* asViewFactory() const {
            return const_cast<ViewFactory*>(static_cast<const ViewFactory*>(this));
        }
    };

    template<typename View>
    struct view: view_helper<view<View>> {

    public:
        template<typename... Args>
        view(Args&&... args): mView(_new<View>(std::forward<Args>(args)...)) {}

        _<View>& operator()() {
            return mView;
        }

        operator _<View>&() {
            return mView;
        }

    private:
        _<View> mView;
    };

    static_assert(std::is_convertible_v<view<AView>, View>, "declarative view wrapper is not convertible to _<AView>");

    template<typename Layout, aui::derived_from<AViewContainer> Container = AViewContainer>
    struct layouted_container_factory_impl {
    private:
        AVector<View> mViews;

    public:
        template<typename... Views>
        layouted_container_factory_impl(Views&&... views) {
            mViews.reserve(sizeof...(views));
            aui::parameter_pack::for_each([this](auto&& item) {
                using type = decltype(item);
                constexpr bool isViewGroup = std::is_convertible_v<type, ViewGroup>;
                constexpr bool isView = std::is_convertible_v<type, View>;
                constexpr bool isInvokable = std::is_invocable_v<type>;

                static_assert(isViewGroup || isView || isInvokable, "the item is neither convertible to View nor ViewGroup, nor invokable");

                if constexpr (isViewGroup) {
                    auto asViewGroup = ViewGroup(item);
                    mViews << std::move(asViewGroup);
                } else if constexpr (isView) {
                    auto asView = View(item);
                    mViews << std::move(asView);
                } else if constexpr(isInvokable) {
                    mViews << item();
                }
            }, std::forward<Views>(views)...);
        }

        _<Container> operator()() {
            auto c = _new<Container>();
            if constexpr(!std::is_same_v<Layout, std::nullopt_t>) {
                c->setLayout(_new<Layout>());
            }
            c->setViews(std::move(mViews));
            return c;
        }
    };


    template<typename Layout, aui::derived_from<AViewContainer> Container = AViewContainer>
    struct layouted_container_factory_impl_with_expanding: layouted_container_factory_impl<Layout, Container> {
    public:
        using layouted_container_factory_impl<Layout, Container>::layouted_container_factory_impl;

        struct Expanding: view_helper<Expanding>, layouted_container_factory_impl<Layout, Container> {
        public:
            template<typename... Views>
            Expanding(Views&&... views): layouted_container_factory_impl<Layout>(std::forward<Views>(views)...) {

            }

            _<Container> operator()() {
                return layouted_container_factory_impl<Layout>::operator()() let {
                    it->setExpanding();
                };
            }
        };
    };

    template <typename Layout, aui::derived_from<AViewContainer> Container = AViewContainer>
    struct layouted_container_factory : view_helper<layouted_container_factory<Layout, Container>>,
                                        layouted_container_factory_impl_with_expanding<Layout, Container> {
        template <typename... Views>
        layouted_container_factory(Views&&... views)
            : layouted_container_factory_impl_with_expanding<Layout, Container>(std::forward<Views>(views)...) {}
    };
}

namespace declarative {

    /**
     * @brief Extra styles wrapper.
     * @ingroup declarative
     * @details
     * @code{cpp}
     * Button { "Default button" },
     * Style{
     *     {
     *         c(".btn"),
     *         BackgroundSolid { 0xff0000_rgb },
     *     },
     * } ({
     *     Button { "Red button" },
     *     Button { "Another red button" },
     * }),
     * @endcode
     */
    struct Style {
    public:
        Style(std::initializer_list<Rule> rules): mStylesheet(_new<AStylesheet>(AStylesheet(rules))) {

        }

        Style& operator()(AVector<_<AView>> views) {
            for (const auto& view : views) {
                AUI_ASSERTX(view->extraStylesheet() == nullptr, "extra stylesheet already specified");
                view->setExtraStylesheet(mStylesheet);
            }
            mViews = std::move(views);
            return *this;
        }

        operator AVector<_<AView>>() noexcept {
            return std::move(mViews);
        }

    private:
        _<AStylesheet> mStylesheet;
        AVector<_<AView>> mViews;
    };
}
