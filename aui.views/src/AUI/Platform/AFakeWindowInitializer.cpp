//
// Created by Alex2772 on 12/7/2021.
//

#include "AFakeWindowInitializer.h"
#include "ABaseWindow.h"


void AFakeWindowInitializer::init(const IRenderingContext::Init& init) {
    IRenderingContext::init(init);
}

void AFakeWindowInitializer::destroyNativeWindow(ABaseWindow& window) {

}

void AFakeWindowInitializer::beginPaint(ABaseWindow& window) {

}

void AFakeWindowInitializer::endPaint(ABaseWindow& window) {

}

void AFakeWindowInitializer::beginResize(ABaseWindow& window) {

}
