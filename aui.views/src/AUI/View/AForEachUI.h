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

namespace aui::detail {
struct InflateOpts {
    bool backward = true;
    bool forward = true;
};
}   // namespace aui::detail

template<typename T>
requires requires(T& t) { std::hash<T>{}(t); }
constexpr std::size_t forEachKey(const T& value) { // default key impl for types with std::hash specialization
    return std::hash<T>{}(value);
}

template<typename T>
requires requires(T& t) { { t.base() } -> ranges::range; }  // default AForEachUIHash specialization for subranges
constexpr std::size_t forEachKey(const T& value) {
    AUI_ASSERT(ranges::begin(value) != ranges::end(value));
    return forEachKey(*ranges::begin(value));
}

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
        std::size_t id;
    };

    using List = aui::dyn_range<Entry>;
    AForEachUIBase() {}
    void setPosition(glm::ivec2 position) override;

    /**
     * @brief Notifies that range was changed or iterators might have invalidated.
     */
    void invalidate();

protected:
    void onViewGraphSubtreeChanged() override;
    void applyGeometryToChildren() override;
    void setModelImpl(List model);

private:
    _<AScrollAreaViewport> mViewport;
    List mViewsModel;
    aui::dyn_range_capabilities mViewsModelCapabilities;
    AOptional<glm::ivec2> mLastInflatedScroll {};

    struct Cache {
        struct LazyListItemInfo : Entry {
            List::iterator iterator;
        };
        AVector<LazyListItemInfo> items;
    };

    AOptional<Cache> mCache;

    void addView(List::iterator iterator, AOptional<std::size_t> index = std::nullopt);
    void removeViews(aui::range<AVector<_<AView>>::iterator> iterators);

    void inflate(aui::detail::InflateOpts opts = {});
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

template <typename T, typename Layout, typename super = AForEachUIBase>
class AForEachUI : public super, public aui::react::DependencyObserver {
public:
    using List = AForEachUIBase::List;
    using ListFactory = std::function<List()>;
    using ViewFactory = std::function<_<AView>(const T& value)>;

    AForEachUI() { this->setLayout(std::make_unique<Layout>()); }

    template <aui::detail::RangeFactory<T> RangeFactory>
    AForEachUI(RangeFactory&& rangeFactory) {
        this->setLayout(std::make_unique<Layout>());
        this->setModel(std::forward<RangeFactory>(rangeFactory));
    }

    template <aui::detail::RangeFactory<T> RangeFactory>
    void setModel(RangeFactory&& rangeFactory) {
        mListFactory = [this, rangeFactory = std::forward<RangeFactory>(rangeFactory)] {
            aui::react::DependencyObserverRegistrar r(*this);
            return rangeFactory() | ranges::views::transform([this](const T& t) {
                       return AForEachUIBase::Entry { .view = mFactory(t), .id = forEachKey(t) };
                   });
        };
    }

    void operator-(ViewFactory f) {
        mFactory = std::move(f);
        updateUnderlyingModel();
    }

    /**
     * @copybrief AForEachUIBase::invalidate
     */
    void invalidate() override { super::invalidate(); }

private:
    ListFactory mListFactory;
    ViewFactory mFactory;

    void updateUnderlyingModel() {
        if (!mFactory) {
            return;
        }

        this->setModelImpl(mListFactory());
    }
};

namespace aui::detail {
template <typename Base /* AForEachUIBase */, typename Layout, aui::invocable RangeFactory>
auto makeForEach(RangeFactory&& rangeFactory)
    requires requires {
        { rangeFactory() } -> ranges::range;
    }
{
    using T = decltype([&] {
        auto rng = rangeFactory();
        return *ranges::begin(rng);
    }());
    return _new<AForEachUI<T, Layout, Base>>(std::forward<RangeFactory>(rangeFactory));
}
}   // namespace aui::detail

/**
 * @see AForEachUIBase
 */
#define AUI_DECLARATIVE_FOR_EX(value, model, layout, ...)                      \
    aui::detail::makeForEach<AForEachUIBase, layout>([&]() -> decltype(auto) { \
        return (model);                                                        \
    }) - [__VA_ARGS__](const auto& value) -> _<AView>

/**
 * @see AForEachUIBase
 */
#define AUI_DECLARATIVE_FOR(value, model, layout) AUI_DECLARATIVE_FOR_EX(value, model, layout, =)
