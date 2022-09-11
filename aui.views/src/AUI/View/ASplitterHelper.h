#pragma once


#include <AUI/Util/ALayoutDirection.h>
#include <AUI/View/AView.h>

class ASplitterHelper {
    friend class ASplitter;
private:
    ALayoutDirection mDirection; // will be initialized in the Builder
    size_t mDraggingDividerIndex = -1;
    int mDragOffset; // may be uninitialized
    AVector<_<AView>> mItems;

    float getTotalOccupiedSizeOf(const _<AView>& view) {
        return mDirection == ALayoutDirection::VERTICAL ? view->getTotalOccupiedHeight() : view->getTotalOccupiedWidth();
    }

    template<typename T>
    [[nodiscard]]
    T& getAxisValue(glm::tvec2<T>& v) {
        switch (mDirection) {
            case ALayoutDirection::VERTICAL  : return v.y;
            case ALayoutDirection::HORIZONTAL: return v.x;
        }
        throw;
    }
    template<typename T>
    [[nodiscard]]
    T getAxisValue(const glm::tvec2<T>& v) {
        switch (mDirection) {
            case ALayoutDirection::VERTICAL  : return v.y;
            case ALayoutDirection::HORIZONTAL: return v.x;
        }
        throw;
    }


public:
    ASplitterHelper() = default;
    ASplitterHelper(ALayoutDirection direction) : mDirection(direction) {}

    void setDirection(ALayoutDirection direction) {
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


