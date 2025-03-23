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

/**
 * @brief Customizable lists display.
 * @ingroup useful_views
 * @details
 * Used to present possibly large or infinite linear non-hiearchial sequences of data.
 *
 * @note
 * If you are familiar with RecyclerView/LazyColumn/LazyRow/LazyVStack/LazyHStack, AForEachUIBase follows the same set
 * of principles; with an exception: AForEachUIBase does not provide a scrollable area on its own.
 *
 * AForEachUIBase is created by using AUI_DECLARATIVE_FOR macro.
 *
 * AUI_DECLARATIVE_FOR mimics *ranged for loop* semantically. AUI_DECLARATIVE_FOR consists of AForEachUIBase creation,
 * single entry variable name, *range* definition and a lambda that creates a new view based on data entry, passed to
 * the lambda. *range* models one-dimensional list.
 *
 * AForEachUIBase works on iterator level by design. Any kind of C++ 20 range/ranges-v3 *range* can be passed to
 * AForEachUIBase. In fact, any kind of *range* can be used, starting from bidirectional containers such as
 * `std::vector` and `std::list`, lazy non-owning dummies like `ranges::views::ints` and even fancy *range* views from
 * `std::ranges::views` or `ranges::views` are acceptable. One-directional containers like `std::queue` or `std::stack`
 * can't be used because they don't implement `begin()` and `end()`. As such, requirements to a *range* are dictated by
 * `ranges::range` concept.
 *
 * - *range* has `.begin()` method or `ranges::begin()` overload defined `auto it = ranges::begin(rng)`
 * - *range* has `.end()` method or `ranges::end()` overload defined `auto it = ranges::end(rng)`
 * - both `begin()` and `end()` return an *iterator*
 * - *iterator* has dereference operator `auto& value = *it`
 * - *iterator* has increment operator `++it`
 *
 * Alternatively, these requirements can be described by *ranged for loop*: `for (const auto& value : rng) { ... }`.
 *
 * The range's type is erased with runtime-based *range* layer @ref aui::dyn_range.
 *
 * AUI_DECLARATIVE_FOR can be nested with no restrictions.
 *
 * # Lazy Semantics
 *
 * AForEachUIBase presents all data available. If placed somewhere inside @ref AScrollArea (implies
 * @ref AScrollAreaViewport), lazy semantics take place. This means that AForEachUIBase knows scroll position and a
 * sliding window size in pixels, making it possible to present a limited set of data that is actually visible, and
 * present data further as soon as the user scrolls down the scroll area.
 *
 * Under the hood, AForEachUIBase stores a pair of iterators of the passed *range* of presented entries, forming a
 * sliding window subrange. When the user scrolls down the list, both iterator are incremented; when the user scrolls
 * upwards, both iterators are decremented.
 *
 * The amount of displayed data is governed by *range* size, render-to-texture tile size, AScrollArea's viewport size
 * and individual entry size. Optimal frequency of sliding and window size are determined by AForEachUIBase. In
 * particular, the sliding is performed once per render-to-texture tile is passed.
 *
 * In lazy scenario, AForEachUIBase adds an additional requirement to range's iterator:
 *
 * - *iterator* has decrement operator `--it`
 */
class API_AUI_VIEWS AForEachUIBase: public AViewContainerBase {
public:
    using List = aui::dyn_range<_<AView>>;
    AForEachUIBase() {}
    void setPosition(glm::ivec2 position) override;
    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;

protected:
    void onViewGraphSubtreeChanged() override;
    void applyGeometryToChildren() override;
    void setModelImpl(List model);

private:
    _<AScrollAreaViewport> mViewport;
    List mViewsModel;
    AOptional<glm::ivec2> mLastInflatedScroll{};
    AOptional<glm::ivec2> mFakeBeginOffset, mFakeEndOffset;

    struct Cache {
        aui::range<List::iterator> inflatedRange;
    };

    AOptional<Cache> mCache;

    void inflateForward();
    void inflateBackward();
    glm::ivec2 calculateOffsetWithinViewportSlidingSurface();
};

template<typename T, typename Layout, typename super = AForEachUIBase>
class AForEachUI: public super {
public:
    using List = aui::dyn_range<_<AView>>;
    using Factory = std::function<_<AView>(const T& value)>;

    AForEachUI() {
        this->setLayout(std::make_unique<Layout>());
    }

    template<ranges::range Rng>
    AForEachUI(Rng&& rng) {
        this->setLayout(std::make_unique<Layout>());
        this->setModel(std::forward<Rng>(rng));
    }

    template<ranges::range Rng>
    requires requires (Rng&& rng) { { *ranges::begin(rng) } -> aui::convertible_to<T>; }
    void setModel(Rng && rng) {
        mDataModel = rng | ranges::views::transform([this](const T& t) { return mFactory(t); });
        updateUnderlyingModel();
    }

    void operator-(Factory f) {
        mFactory = std::move(f);
        updateUnderlyingModel();
    }

    [[nodiscard]]
    const List& model() const noexcept {
        return mDataModel;
    }

private:
    List mDataModel;
    Factory mFactory;

    void updateUnderlyingModel() {
        if (!mFactory) {
            return;
        }

        this->setModelImpl(mDataModel);
    }

};

namespace aui::detail {
template<typename Base /* AForEachUIBase */, typename Layout, ranges::range Rng>
auto makeForEach(Rng&& rng) {
    using T = std::decay_t<decltype(*ranges::begin(rng))>;
    return _new<AForEachUI<T, Layout, Base>>(std::forward<Rng>(rng));
}
}   // namespace aui::detail

/**
 * @see AForEachUIBase
 */
#define AUI_DECLARATIVE_FOR_EX(value, model, layout, ...) aui::detail::makeForEach<AForEachUIBase, layout>(model) - [__VA_ARGS__](const auto& value) -> _<AView>

/**
 * @see AForEachUIBase
 */
#define AUI_DECLARATIVE_FOR(value, model, layout) AUI_DECLARATIVE_FOR_EX(value, model, layout, =)
