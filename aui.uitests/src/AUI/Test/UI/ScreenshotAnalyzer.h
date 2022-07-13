#pragma once

#include <AUI/Image/AImage.h>
#include "AUI/Common/AColor.h"
#include "AUI/View/AView.h"

class API_AUI_UITESTS ScreenshotAnalyzer {
public:
    static ScreenshotAnalyzer makeScreenshot();

    [[nodiscard]]
    AColor averageColor() const noexcept {
        return mImage.averageColor();
    }

    ScreenshotAnalyzer clip(const _<AView>& view) const {
        return clip(view->getPositionInWindow(), view->getSize());
    }
    ScreenshotAnalyzer clip(glm::ivec2 position, glm::ivec2 size) const {
        return mImage.sub(position, size);
    }

private:
    AImage mImage;

    ScreenshotAnalyzer(AImage image) : mImage(std::move(image)) {}
};


