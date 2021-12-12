//
// Created by Alex2772 on 12/7/2021.
//

#include "AFakeWindowInitializer.h"



void AFakeWindowInitializer::init(const IRenderingContext::Init& init) {
    IRenderingContext::init(init);
}

void AFakeWindowInitializer::destroyNativeWindow(AWindow& window) {

}

void AFakeWindowInitializer::beginPaint(AWindow& window) {

}

void AFakeWindowInitializer::endPaint(AWindow& window) {

}

void AFakeWindowInitializer::beginResize(AWindow& window) {

}
