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

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Util/LayoutDirection.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include "AViewContainer.h"
#include "AHDividerView.h"
#include "AVDividerView.h"

class API_AUI_VIEWS ASplitter: public AViewContainer
{
private:
    template<typename Layout>
    friend class Builder;

    LayoutDirection mDirection;
    std::function<_<AView>()> mDividerFactory;
    AVector<_<AView>> mItems;
    AVector<_<AView>> mDividers;
    unsigned mDraggingDividerIndex = -1;
    int mDragOffset;

    ASplitter();

    void updateSplitterItems();

    float getTotalOccupiedSizeOf(const _<AView>& view) {
        return mDirection == LayoutDirection::VERTICAL ? view->getTotalOccupiedHeight() : view->getTotalOccupiedWidth();
    }

    template<typename T>
    [[nodiscard]]
    T& getAxisValue(glm::tvec2<T>& v) {
        switch (mDirection) {
            case LayoutDirection::VERTICAL  : return v.y;
            case LayoutDirection::HORIZONTAL: return v.x;
        }
        throw;
    }
    template<typename T>
    [[nodiscard]]
    T getAxisValue(const glm::tvec2<T>& v) {
        switch (mDirection) {
            case LayoutDirection::VERTICAL  : return v.y;
            case LayoutDirection::HORIZONTAL: return v.x;
        }
        throw;
    }

    template<typename Layout>
    class Builder {
    friend class ASplitter;
    private:
        std::function<_<AView>()> mDividerFactory = []() -> _<AView> {
            if constexpr (Layout::DIRECTION == LayoutDirection::VERTICAL) {
                return _new<AHDividerView>();
            }
            return _new<AVDividerView>();
        };
        AVector<_<AView>> mItems;

    public:
       Builder& withItems(const AVector<_<AView>>& items) {
           mItems = items;
           return *this;
       }

       Builder& withDivider(const std::function<_<AView>()>& factory) {
           mDividerFactory = factory;
           return *this;
       }

       template<typename T>
       Builder& withDivider() {
           static_assert(std::is_base_of_v<AView, T>, "use instance of AView in withDivider");
           withDivider([] {
              return _new<T>();
           });
           return *this;
       }

       _<AView> build() {
           _<ASplitter> splitter = new ASplitter;
           splitter->mDividerFactory = std::move(mDividerFactory);
           splitter->mItems = std::move(mItems);
           splitter->mDirection = Layout::DIRECTION;
           splitter->updateSplitterItems();
           return splitter;
       }

       operator _<AView>() {
           return build();
       }
    };

public:
    virtual ~ASplitter() = default;

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    [[nodiscard]]
    bool isDragging() const {
        return mDraggingDividerIndex != -1;
    }

    void onMouseMove(glm::ivec2 pos) override;

    void setSize(int width, int height) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    using Vertical = Builder<AVerticalLayout>;
    using Horizontal = Builder<AHorizontalLayout>;

};
