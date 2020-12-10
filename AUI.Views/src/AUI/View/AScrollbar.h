//
// Created by alex2 on 06.12.2020.
//

#pragma once

#include <AUI/Util/LayoutDirection.h>
#include "AViewContainer.h"
#include "ASpacer.h"

class AScrollbarHandle;
class AScrollbarButton;

class API_AUI_VIEWS AScrollbar: public AViewContainer {
private:
    LayoutDirection mDirection;
    _<ASpacer> mOffsetSpacer;
    _<AScrollbarHandle> mHandle;
    _<AScrollbarButton> mForwardButton;
    _<AScrollbarButton> mBackwardButton;

    size_t mViewportSize = 0, mFullSize = 0;
    int mCurrentScroll = 0;

    void setOffset(size_t o);

public:

    explicit AScrollbar(LayoutDirection direction);

    void setScrollDimensions(size_t viewportSize, size_t fullSize);

    void updateScrollHandleSize();
    void setScroll(int scroll);
    void onMouseWheel(glm::ivec2 pos, int delta) override;

signals:
    emits<int> scrolled;
};


