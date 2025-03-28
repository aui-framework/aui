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

#include <range/v3/view/transform.hpp>

#include "AScrollAreaViewport.h"
#include "AViewContainer.h"
#include <AUI/Model/IListModel.h>
#include <functional>
#include <AUI/Model/AListModelAdapter.h>
#include <AUI/Util/ADataBinding.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Traits/dyn_range.h>

namespace aui::for_each_ui {

/**
 * @brief Key, used to avoid view instantiation on AForEachUI::setModelImpl.
 * @details
 * Typically consists of a hash of underlying value.
 */
using Key = std::size_t;

template <typename T>
    requires requires(T& t) { std::hash<T> {}(t); }
constexpr aui::for_each_ui::Key defaultKey(const T& value, long primaryCandidate) {   // std::hash specialization
    return std::hash<T> {}(value);
}

template <typename T>
constexpr aui::for_each_ui::Key defaultKey(const _<T>& value, int secondaryCandidate) {   // specialization for shared pointers
    return reinterpret_cast<std::uintptr_t>(value.get());
}

template <ranges::input_range T>
constexpr aui::for_each_ui::Key defaultKey(const T& value, int secondaryCandidate) {   // specialization for subranges
    aui::for_each_ui::Key k{};
    for (const auto& i : value) {
        k ^= defaultKey(i, 0L);
    }
    return k;
}

namespace detail {
struct InflateOpts {
    bool backward = true;
    bool forward = true;
};
using ViewsSharedCache = AMap<aui::for_each_ui::Key, _<AView>>;
}
}   // namespace aui::for_each_ui

/**
 * @brief Customizable lists display.
 * @ingroup useful_views
 * @details
 * Used to lazily present possibly large or infinite linear non-hierarchical sequences of data.
 *
 * @note
 * If you are familiar with RecyclerView/LazyColumn/LazyRow/LazyVStack/LazyHStack, AForEachUIBase follows the same set
 * of principles; with an exception: AForEachUIBase does not provide a scrollable area on its own.
 *
 * AForEachUIBase is created by using AUI_DECLARATIVE_FOR macro.
 *
 * AUI_DECLARATIVE_FOR mimics *ranged for loop* semantically.
 *
 * @code{cpp}
 * static std::array users = { "Foo", "Bar", "Lol" };
 * for (const auto& user : users) {
 *   fmt::println("{}", user);
 * }
 * @endcode
 * @code{cpp}
 * static std::array users = { "Foo", "Bar", "Lol" };
 * ...
 * AUI_DECLARATIVE_FOR(user, users, AVerticalLayout) {
 *   return Label { fmt::format("{}", user) };
 * }
 * @endcode
 *
 * `AUI_DECLARATIVE_FOR` consists of AForEachUIBase creation, single entry variable name, *range* definition, layout
 * name (acceptable are `AVerticalLayout` and `AHorizontalLayout`) and a lambda that creates a new view based on data
 * entry. In terms of C++ syntax, the lambda is partially defined by `AUI_DECLARATIVE_FOR` macro; the lambda's body
 * (including curly braces) is left up to developer.
 *
 * *range* models one-dimensional list.
 *
 * AForEachUIBase works on iterator level by design. In fact, any kind of *range* (C++20 ranges/range-v3) can be used,
 * starting from bidirectional containers such as `std::vector` and `std::list`, lazy non-owning dummies like
 * `ranges::views::ints` and even fancy *range* views from `std::ranges::views` or `ranges::views` are acceptable.
 * One-directional containers like `std::queue` or `std::stack` can't be used because they don't implement `begin()` and
 * `end()`. As such, requirements to a *range* are dictated by `ranges::range` concept.
 *
 * - *range* has `.begin()` method or `ranges::begin()` overload defined `auto it = ranges::begin(rng)`
 * - *range* has `.end()` method or `ranges::end()` overload defined `auto it = ranges::end(rng)`
 * - both `begin()` and `end()` return an *iterator*
 * - *iterator* has dereference operator `auto& value = *it`
 * - *iterator* has increment operator `++it`
 *
 * Alternatively, these requirements can be described by a *ranged for loop*: `for (const auto& value : rng) { ... }`.
 *
 * The range's type is erased with runtime-based *range* layer @ref aui::dyn_range.
 *
 * `AUI_DECLARATIVE_FOR` can be nested with no restrictions. When using the same dataset in nested declarative *for
 * loops*, a hierarchical structure can be presented. For example, we can split a sorted array of people names by first
 * letter to produce a labelled index:
 *
 * @snippet examples/ui/contacts/src/main.cpp NESTED_FOR_EXAMPLE
 *
 * (@ref example_contacts example)
 *
 * # Lazy Semantics
 *
 * AForEachUIBase presents all data available. If placed somewhere inside @ref AScrollArea (implies
 * @ref AScrollAreaViewport), lazy semantics take place. This means that AForEachUIBase knows scroll position and
 * sliding window size in pixels, making it possible to present a limited set of data that is actually visible, and
 * present data further as soon as the user scrolls down the scroll area.
 *
 * Under the hood, AForEachUIBase stores a pair of iterators of the passed *range* of presented entries, forming a
 * sliding window subrange. When the user scrolls down the list, both iterators are incremented; when the user scrolls
 * upwards, both iterators are decremented.
 *
 * In this scenario, AForEachUIBase adds an extra requirement to range's iterator:
 *
 * - *iterator* has decrement operator `--it`
 *
 * If this requirement is not satisfied (case of some `ranges::views`), AForEachUIBase would not unload old items,
 * unless a @ref AFOREACHUI_UPDATE "data update event" occurred.
 *
 * The amount of displayed data is governed by *range* size, @ref "docs/Render to texture.md" tile size, AScrollArea's
 * viewport size and individual entry size. Optimal frequency of sliding during scroll and window size are determined by
 * AForEachUIBase. In particular, the sliding is performed once per @ref "docs/Render to texture.md" tile is passed.
 *
 * @note
 * During rendering inside AScrollArea, the renderer clips visible views more precisely; the goal of lazy semantics of
 * AForEachUIBase is to optimize view instantiation and layout processing overhead, as well as *range* views' lazy
 * semantics, thanks to iterators.
 *
 * ## Scrollbars
 *
 * From perspective of layout, lazy semantics is implemented by careful layout updates driven by scroll area events. If
 * possible, the items that appear far from sliding window are unloaded (views are removed). The new items are loaded
 * (new views are instantiated). To avoid content jittering, scroll position is synced with layout updates within
 * AForEachUIBase. As such, these hijacking operations may confuse scroll bar.
 *
 * In modern software, especially when it comes to infinite lists in web/mobile applications (i.e., news feed),
 * scrollbar might be completely hidden or significantly transparentized.
 *
 * This optimization gives a severe performance benefit. Despite the fact that there's a complete mess "under the hood"
 * (scrollbar is the only visual confirmation), the scrolled contents appears normal and natural.
 *
 * @image html docs/imgs/edrfgsrgsrg.webp A lie is going on behind the scenes
 *
 * # List Updates {#AFOREACHUI_UPDATE}
 */
class API_AUI_VIEWS AForEachUIBase : public AViewContainerBase {
public:
    struct Entry {
        _<AView> view;
        aui::for_each_ui::Key id;
    };

    using List = aui::dyn_range<Entry>;
    AForEachUIBase() {}
    void setPosition(glm::ivec2 position) override;

protected:
    struct Cache {
        struct LazyListItemInfo : Entry {
            List::iterator iterator;
        };
        AVector<LazyListItemInfo> items;
    };

    AOptional<Cache> mCache;

    void onViewGraphSubtreeChanged() override;
    void applyGeometryToChildren() override;

    /**
     * @brief Notifies that range was changed or iterators might have invalidated.
     */
    void setModelImpl(List model);

    /**
     * @brief Returns a cache of views, if any.
     * @details
     * Called by `setModelImpl`. The implementation might then use the shared cache to return views from `List` range.
     *
     * The implementation is responsible to clean up the cache.
     */
    virtual aui::for_each_ui::detail::ViewsSharedCache* getViewsCache() = 0;

private:
    _<AScrollAreaViewport> mViewport;
    List mViewsModel;
    aui::dyn_range_capabilities mViewsModelCapabilities;
    AOptional<glm::ivec2> mLastInflatedScroll {};

    void addView(List::iterator iterator, AOptional<std::size_t> index = std::nullopt);
    void removeViews(aui::range<AVector<_<AView>>::iterator> iterators);

    void inflate(aui::for_each_ui::detail::InflateOpts opts = {});
    glm::ivec2 calculateOffsetWithinViewportSlidingSurface();
    glm::ivec2 axisMask();
};

namespace aui::detail {
template <typename Factory, typename T>
concept RangeFactory = requires(Factory&& factory) {
    { factory } -> aui::invocable;
    { factory() } -> aui::range_consisting_of<T>;
};
}   // namespace aui::detail

template <typename T>
class AForEachUI : public AForEachUIBase, public aui::react::DependencyObserver {
public:
    using List = AForEachUIBase::List;
    using ListFactory = std::function<List()>;
    using ViewFactory = std::function<_<AView>(const T& value)>;

    AForEachUI() {}

    template <aui::detail::RangeFactory<T> RangeFactory>
    AForEachUI(RangeFactory&& rangeFactory) {
        this->setModel(std::forward<RangeFactory>(rangeFactory));
    }

    template <aui::detail::RangeFactory<T> RangeFactory>
    void setModel(RangeFactory&& rangeFactory) {
        mListFactory = [this, rangeFactory = std::forward<RangeFactory>(rangeFactory)] {
            aui::react::DependencyObserverRegistrar r(*this);
            return rangeFactory() | ranges::views::transform([this](const T& t) {
                       auto key = aui::for_each_ui::defaultKey(t, 0L);
                       _<AView> view;
                       if (mViewsSharedCache) {
                           if (auto c = mViewsSharedCache->contains(key)) {
                               view = std::exchange(c->second, nullptr);
                           }
                       }
                       if (!view) {
                           view = mFactory(t);
                       }
                       return AForEachUIBase::Entry { .view = std::move(view), .id = key };
                   });
        };
    }

    /**
     * @internal
     * @brief Helper function for AUI_DECLARATIVE_FOR
     */
    template<aui::invocable<const T&> ViewFactoryT>
    void operator-(ViewFactoryT&& f) {
        mFactory = std::forward<ViewFactoryT>(f);
        mViewsSharedCache = &VIEWS_SHARED_CACHE<ViewFactoryT>;
        updateUnderlyingModel();
    }

    /**
     * @copybrief AForEachUIBase::setModelImpl
     */
    void invalidate() override { updateUnderlyingModel(); }

    using AViewContainerBase::setLayout;

protected:
    aui::for_each_ui::detail::ViewsSharedCache* getViewsCache() override {
        return mViewsSharedCache;
    }

private:
    template <typename FactoryTypeTag>
    static aui::for_each_ui::detail::ViewsSharedCache VIEWS_SHARED_CACHE;

    aui::for_each_ui::detail::ViewsSharedCache* mViewsSharedCache = nullptr;
    ListFactory mListFactory;
    ViewFactory mFactory;

    void updateUnderlyingModel() {
        if (!mFactory) {
            return;
        }

        this->setModelImpl(mListFactory());
    }
};

template<typename T>
template<typename FactoryTypeTag>
aui::for_each_ui::detail::ViewsSharedCache AForEachUI<T>::VIEWS_SHARED_CACHE{};

namespace aui::detail {
template <typename Layout, aui::invocable RangeFactory>
auto makeForEach(RangeFactory&& rangeFactory)
    requires requires {
        { rangeFactory() } -> ranges::range;
    }
{
    using T = decltype([&] {
        auto rng = rangeFactory();
        return *ranges::begin(rng);
    }());

    auto result = _new<AForEachUI<T>>(std::forward<RangeFactory>(rangeFactory));
    result->setLayout(std::make_unique<Layout>());
    return result;
}
}   // namespace aui::detail

/**
 * @see AForEachUIBase
 */
#define AUI_DECLARATIVE_FOR_EX(value, model, layout, ...)                      \
    aui::detail::makeForEach<layout>([&]() -> decltype(auto) { \
        return (model);                                                        \
    }) - [__VA_ARGS__](const auto& value) -> _<AView>

/**
 * @see AForEachUIBase
 */
#define AUI_DECLARATIVE_FOR(value, model, layout) AUI_DECLARATIVE_FOR_EX(value, model, layout, =)
