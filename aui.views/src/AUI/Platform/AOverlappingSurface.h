#pragma once


#include <AUI/View/AViewContainer.h>

class ABaseWindow;

class AOverlappingSurface: public AViewContainer {
    friend class ABaseWindow;
private:
    ABaseWindow* mParentWindow;
    bool mCloseOnClick;

public:
    AOverlappingSurface() = default;

    virtual ~AOverlappingSurface() = default;
    void close();

    bool isCloseOnClick() const {
        return mCloseOnClick;
    }

    void setCloseOnClick(bool closeOnClick) {
        mCloseOnClick = closeOnClick;
    }

    virtual void setOverlappingSurfacePosition(glm::ivec2 position) = 0;
};