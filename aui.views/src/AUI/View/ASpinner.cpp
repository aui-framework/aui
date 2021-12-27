//
// Created by Alex2772 on 12/27/2021.
//

#include "ASpinner.h"

#include <AUI/Render/Render.h>

void ASpinner::render() {
    using namespace std::chrono;
    Render::translate(glm::vec2(getSize()) / 2.f);
    Render::rotate(float(
            duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() % 1000) / 1000.f * 2 * glm::pi<float>());
    Render::translate(-glm::vec2(getSize()) / 2.f);
    AView::render();
    redraw();
}
