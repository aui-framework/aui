//
// Created by Alex2772 on 7/8/2022.
//

#include "ScreenshotAnalyzer.h"
#include "AUI/Platform/AWindow.h"

ScreenshotAnalyzer ScreenshotAnalyzer::makeScreenshot() {
    return AWindow::current()->getRenderingContext()->makeScreenshot();
}

