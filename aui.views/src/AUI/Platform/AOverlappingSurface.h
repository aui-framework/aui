#pragma once


#include <AUI/View/AViewContainer.h>

class ABaseWindow;

class API_AUI_VIEWS AOverlappingSurface: public AViewContainer {
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
    virtual void setOverlappingSurfaceSize(glm::ivec2 size) = 0;
};