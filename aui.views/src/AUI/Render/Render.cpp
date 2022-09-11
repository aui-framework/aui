//
// Created by Alex2772 on 11/19/2021.
//

#include <AUI/Util/ACleanup.h>
#include <AUI/Util/kAUI.h>
#include "Render.h"


_unique<IRenderer> Render::ourRenderer;

void Render::setRenderer(_unique<IRenderer> renderer) {
    ourRenderer = std::move(renderer);
    do_once ACleanup::afterEntry([] {
        ourRenderer = nullptr;
    });
}
