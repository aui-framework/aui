#pragma once


#include <AUI/Util/LayoutDirection.h>
#include <AUI/View/AView.h>

class ASplitterHelper {
    friend class ASplitter;
private:
    LayoutDirection mDirection; // will be initialized in the Builder
    size_t mDraggingDividerIndex = -1;
    int mDragOffset; // may be uninitialized
    AVector<_<AView>> mItems;

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


public:
    ASplitterHelper() = default;
    ASplitterHelper(LayoutDirection direction) : mDirection(direction) {}

    void setDirection(LayoutDirection direction) {
        mDirection = direction;
    }

    void beginDrag(const glm::ivec2& mousePos);
    bool mouseDrag(const glm::ivec2& mousePos);
    void endDrag() {
        mDraggingDividerIndex = -1;
    }

    void setItems(AVector<_<AView>>&& items) {
        mItems = items;
    }

    [[nodiscard]]
    bool isDragging() const {
        return mDraggingDividerIndex != -1;
    }

};


