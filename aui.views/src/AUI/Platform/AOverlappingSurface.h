#pragma once


#include <AUI/View/AViewContainer.h>

class ABaseWindow;

class AOverlappingSurface: public AViewContainer {
    friend class ABaseWindow;
private:
    ABaseWindow* mWindow;
    bool mCloseOnClick;

public:
    AOverlappingSurface() {}

    virtual ~AOverlappingSurface() = default;
    void close();

    bool isCloseOnClick() const {
        return mCloseOnClick;
    }

    void setCloseOnClick(bool closeOnClick) {
        mCloseOnClick = closeOnClick;
    }
};