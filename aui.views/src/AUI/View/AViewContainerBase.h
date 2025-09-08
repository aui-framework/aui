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

#include <AUI/Views.h>
#include <AUI/View/AView.h>
#include "AUI/Common/SharedPtr.h"
#include "AUI/Enum/AOverflow.h"
#include "AUI/Util/ABitField.h"
#include <glm/glm.hpp>
#include "AUI/Layout/ALayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Render/ARenderContext.h"
#include "glm/fwd.hpp"


AUI_ENUM_FLAG(AViewLookupFlags) {
    NONE = 0,

    /**
     * @brief On AViewContainerBase::getViewAt*, ignores visibility flags.
     */
    IGNORE_VISIBILITY = 0b1,

    /**
     * @brief On AViewContainerBase::getViewAt* with callbacks, only the first view in the container is passed to callback.
     * @details
     * Basically this flag allows to replicate mouse click handling behaviour, which is useful in creating custom events
     * (i.e. drag&drop).
     */
    ONLY_ONE_PER_CONTAINER = 0b10,

    /**
     * @brief Match views only by consumesClick() property.
     * @details
     * By default, getViewAt returns the first hit view, if neither view of the container have not passed
     * consumesClick test. This flag disables that behaviour, returning nullptr instead.
     */
    ONLY_THAT_CONSUMES_CLICK = 0b100,
};

/**
 * @brief A view that represents a set of views.
 * @details
 * AViewContainerBase can store, render, resize, provide events to and handle the child views.
 *
 * AViewContainerBase does not control the position and size of the child views by itself; instead, it delegates that
 * responsibility to it's [layout manager](layout-managers.md).
 *
 * Since AViewContainerBase is an instance of AView, AViewContainerBase can handle AViewContainerBases recursively, thus, making
 * possible complex UI by nested AViewContainerBases with different layout managers.
 *
 * This class is abstract. The methods for modifying it's views are protected. You can use AViewContainer which exposes
 * these methods to public or extend AViewContainerBase to make your own container-like view.
 */
class API_AUI_VIEWS AViewContainerBase : public AView {
public:
    friend class AView;

    struct PointerEventsMapping {
        APointerIndex pointerIndex;
        _weak<AView> targetView;
        /**
         * @brief true if the view or any child (direct or indirect) of the view blocks clicks when pressed
         * @see mBlockClicksWhenPressed
         */
        bool isBlockClicksWhenPressed = true;
    };

    AViewContainerBase();
    ~AViewContainerBase() override = 0; // this class is intended to be derived; use AViewContainer for trivial
                                        // container



    void render(ARenderContext context) override;

    void onMouseEnter() override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;

    void onMouseLeave() override;

    void onDpiChanged() override;

    void onClickPrevented() override;

    int getContentMinimumWidth() override;

    int getContentMinimumHeight() override;

    void onPointerPressed(const APointerPressedEvent& event) override;

    void onPointerDoubleClicked(const APointerPressedEvent& event) override;

    void onPointerReleased(const APointerReleasedEvent& event) override;

    void onScroll(const AScrollEvent& event) override;

    bool onGesture(const glm::ivec2& origin, const AGestureEvent& event) override;

    bool consumesClick(const glm::ivec2& pos) override;

    void setSize(glm::ivec2 size) override;

    void setEnabled(bool enabled = true) override;

    auto begin() const {
        return mViews.cbegin();
    }

    auto end() const {
        return mViews.cend();
    }

    /**
     * @brief Get all views of the container.
     */
    const AVector<_<AView>>& getViews() const {
        return mViews;
    }


    /**
     * @brief Get layout manager of the container.
     */
    [[nodiscard]]
    const _unique<ALayout>& getLayout() const noexcept {
        return mLayout;
    }

    /**
     * @brief Finds first direct child view under position.
     * @param pos position relative to this container
     * @param flags see AViewLookupFlags
     * @return found view or nullptr
     * @details
     * Some containers may implement getViewAt by it's own (i.e. AListView for performance reasons).
     */
    [[nodiscard]]
    virtual _<AView> getViewAt(glm::ivec2 pos, ABitField<AViewLookupFlags> flags = AViewLookupFlags::NONE) const noexcept;

    /**
     * @brief Acts as AViewContainerBase::getViewAt but recursively (may include non-direct child).
     * @param pos position relative to this container
     * @param flags see AViewLookupFlags
     * @return found view or nullptr
     */
    [[nodiscard]]
    _<AView> getViewAtRecursive(glm::ivec2 pos, ABitField<AViewLookupFlags> flags = AViewLookupFlags::NONE) const noexcept;

    /**
     * @brief Acts as AViewContainerBase::getViewAtRecursive but calls a callback instead of returning value.
     * @param pos position relative to this container
     * @param callback callback to call
     * @param flags see AViewLookupFlags
     * @return true if callback returned true; false otherwise
     * @details
     * The passed callback is a predicate. If predicate returns true, the execution of lookup is stopped and getViewAt
     * returns true. If predicate returns false, the lookup continues.
     */
    template<aui::predicate<_<AView>> Callback>
    bool getViewAtRecursive(glm::ivec2 pos, const Callback& callback, ABitField<AViewLookupFlags> flags = AViewLookupFlags::NONE) {
        _<AView> possibleOutput; // for case if anyone does not consumesClick
        auto process = [&](const _<AView>& view) {
            if (callback(view))
                return true;
            if (auto container = _cast<AViewContainerBase>(view)) {
                if (container->getViewAtRecursive(pos - view->getPosition(), callback, flags)) {
                    return true;
                }
            }
            return false;
        };

        for (const auto& view : aui::reverse_iterator_wrap(mViews)) {
            auto targetPos = pos - view->getPosition();

            if (targetPos.x < 0 || targetPos.y < 0 || targetPos.x >= view->getSize().x || targetPos.y >= view->getSize().y) {
                continue;
            }
            if (!flags.test(AViewLookupFlags::IGNORE_VISIBILITY) && !(view->getVisibility() & Visibility::FLAG_CONSUME_CLICKS)) {
                continue;
            }

            if (view->consumesClick(targetPos)) {
                if (flags.test(AViewLookupFlags::IGNORE_VISIBILITY) || !!(view->getVisibility() & Visibility::FLAG_CONSUME_CLICKS)) {
                    if (process(view)) {
                        return true;
                    }

                    if (flags.test(AViewLookupFlags::ONLY_ONE_PER_CONTAINER)) {
                        return false;
                    }
                }
            } else {
                if (possibleOutput == nullptr) {
                    possibleOutput = view;
                }
            }
        }
        if (possibleOutput) {
            return process(possibleOutput);
        }

        return false;
    }

    /**
     * @brief Performs recursive view traversal.
     * @param callback callback to call. Returns true to finish traversal.
     * @param flags see AViewLookupFlags
     * @return true if there's at least one view for which callback returned true, false otherwise.
     */
    template<aui::predicate<_<AView>> Callback>
    bool visitsViewRecursive(Callback&& callback, ABitField<AViewLookupFlags> flags = AViewLookupFlags::NONE) {
        for (auto it = mViews.rbegin(); it != mViews.rend(); ++it) {
            auto view = *it;
            if (flags.test(AViewLookupFlags::IGNORE_VISIBILITY) || !!(view->getVisibility() & Visibility::FLAG_CONSUME_CLICKS)) {
                if (callback(view))
                    return true;
                if (auto container = _cast<AViewContainerBase>(view)) {
                    if (container->visitsViewRecursive(callback, flags)) {
                        return true;
                    }
                }
                if (flags.test(AViewLookupFlags::ONLY_ONE_PER_CONTAINER)) {
                    break;
                }
            }
        }
        return false;
    }


    /**
     * @brief Acts as AViewContainerBase::getViewAtRecursive but finds a view castable to specified template type.
     * @param pos position relative to this container
     * @param flags see AViewLookupFlags
     * @return found view or nullptr
     */
    template<typename T>
    _<T> getViewAtRecursiveOfType(glm::ivec2 pos, ABitField<AViewLookupFlags> flags = AViewLookupFlags::NONE) {
        _<T> result;
        getViewAtRecursive(pos, [&] (const _<AView>& v) { return bool(result = _cast<T>(v)); }, flags);
        return result;
    }

    /**
     * @brief Set focus chain target.
     * @param target a new focus chain view belonging to this container
     * @details
     * See mFocusChainTarget for further info.
     */
    void setFocusChainTarget(_weak<AView> target) {
        if (auto v = target.lock()) {
            AUI_ASSERT(v->mParent == this);
        }
        mFocusChainTarget = std::move(target);
    }
    /**
     * @return Focus chain target.
     * @details
     * See mFocusChainTarget for further info.
     */
    _<AView> focusChainTarget() {
        if (auto v = mFocusChainTarget.lock()) {
            if (v->mParent != this) {
                mFocusChainTarget.reset();
                return nullptr;
            }
            return v;
        }
        return nullptr;
    }


    /**
     * @brief Applies geometry to all children if needed.
     * @details
     * See [layout-managers] for more info.
     */
    void applyGeometryToChildrenIfNecessary();

    void onKeyDown(AInput::Key key) override;

    void onKeyRepeat(AInput::Key key) override;

    void onKeyUp(AInput::Key key) override;

    void onCharEntered(AChar c) override;

    bool capturesFocus() override;

    /**
     * @see mPointerEventsMapping
     */
    const ASmallVector<PointerEventsMapping, 1>& pointerEventsMapping() const noexcept {
        return mPointerEventsMapping;
    }

    void forceUpdateLayoutRecursively() override;

    void markMinContentSizeInvalid() override;
    void markPixelDataInvalid(ARect<int> invalidArea) override;

protected:
    bool mWantsLayoutUpdate = true;
    glm::ivec2 mLastLayoutUpdateSize{0, 0};

    void drawView(const _<AView>& view, ARenderContext contextOfTheContainer);

    template<typename Iterator>
    void drawViews(Iterator begin, Iterator end, ARenderContext contextPassedToContainer) {
        switch (mOverflow) {
            case AOverflow::VISIBLE: break;
            case AOverflow::HIDDEN:
            case AOverflow::HIDDEN_FROM_THIS:
                contextPassedToContainer.clip(ARect<int>{
                    .p1 = {0, 0},
                    .p2 = getSize(),
                });
        }

        std::unique_lock lock(mViewsSafeIteration, std::try_to_lock);
        if (!lock) {
            throw AException("drawViews: can't ensure safe iteration");
        }
        for (auto i = begin; i != end; ++i) {
            drawView(*i, contextPassedToContainer);
        }
    }

    void invalidateAllStyles() override;
    void onViewGraphSubtreeChanged() override;
    void invalidateAssHelper() override;

    /**
     * @brief Replace views.
     */
    void setViews(AVector<_<AView>> views);

    /**
     * @brief Adds view to container without exposing it to the layout manager.
     * @details
     * User is obligated to manage view's layout by themselves. Implement applyGeometryToChildren() to do so.
     *
     * View is not visible until it's layout is determined. @see AView::mSkipUntilLayoutUpdate
     */
    void addViewCustomLayout(const _<AView>& view);

    /**
     * @brief Add all views from vector.
     */
    void addViews(AVector<_<AView>> views);

    /**
     * @brief Add view to the container.
     */
    void addView(const _<AView>& view);

    /**
     * @brief Add view at specific index to the container.
     */
    void addView(size_t index, const _<AView>& view);

    /**
     * @brief Remove views from the container.
     */
    void removeViews(aui::range<AVector<_<AView>>::const_iterator> views);

    /**
     * @brief Remove view from the container.
     */
    void removeView(const _<AView>& view);

    /**
     * @brief Remove view from the container.
     */
    void removeView(AView* view);

    /**
     * @brief Remove view from the container at specified index.
     */
    void removeView(size_t index);

    /**
     * @brief Remove all views from container.
     */
    void removeAllViews();

    /**
     * @brief Moves (like via std::move) all children and layout of the specified container to this container.
     * @param container container. Must be pure AViewContainer (cannot be a derivative from AViewContainer).
     */
    void setContents(const _<AViewContainer>& container);

    /**
     * @brief Set new layout manager for this AViewContainerBase. DESTROYS OLD LAYOUT MANAGER WITH ITS VIEWS!!!
     */
    void setLayout(_unique<ALayout> layout);

    void renderChildren(ARenderContext contextPassedToContainer) {
        drawViews(mViews.begin(), mViews.end(), contextPassedToContainer);
    }

    /**
     * @brief Applies geometry to all children with no preconditions.
     * @details
     * See [layout-managers] for more info.
     */
    virtual void applyGeometryToChildren();

signals:
    /**
     * @brief Emitted when addView(s)/removeView/setLayout was called.
     */
    emits<> childrenChanged;

private:
    _unique<ALayout> mLayout;
    ASpinlockMutex mViewsSafeIteration;
    AVector<_<AView>> mViews;
    bool mSizeSet = false;


    struct RepaintTrap {
        bool triggered = false;
    };
    AOptional<RepaintTrap> mRepaintTrap;

    struct ConsumesClickCache {
        glm::ivec2 position;
        bool value;
    };

    /**
     * @brief Temporary cache for consumesClick().
     * @details
     * AUI does several view graph visits for a single event in order to call virtual methods properly.
     * AViewContainerBase::consumesClick() is expensive method. Thus, we can cache it's result with given arguments.
     */
    AOptional<ConsumesClickCache> mConsumesClickCache;

    /**
     * @brief Focus chain target.
     * @details
     * Focus chain target is a view belonging to this container which focus-aware (i.e. keyboard) events are passed to.
     * The focus chaining mechanism allows to catch such events and process them in the containers.
     */
    _weak<AView> mFocusChainTarget;

    /**
     * @brief Like focus chain target, but intended for pointer press -> move.. -> release event sequence on per-pointer
     * (finger) basis.
     */
    ASmallVector<PointerEventsMapping, 1> mPointerEventsMapping;

    void notifyParentEnabledStateChanged(bool enabled) override;
    void invalidateCaches();

    /**
     * @see mPointerEventsMapping
     */
    _<AView> pointerEventsMapping(APointerIndex index);

    void removeViewImpl(const _<AView>& view, std::unique_lock<ASpinlockMutex>& lock);
    void setLayoutImpl(_unique<ALayout> layout, std::unique_lock<ASpinlockMutex>& lock);
    void removeAllViewsImpl(std::unique_lock<ASpinlockMutex>& lock);
};
