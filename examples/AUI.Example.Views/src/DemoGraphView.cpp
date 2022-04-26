#include "DemoGraphView.h"
#include <AUI/ASS/ASS.h>

constexpr auto POINT_COUNT = 100;

DemoGraphView::DemoGraphView()
{
    setFixedSize({ 5_dp * POINT_COUNT, 100_dp }); // set fixed size


    mPoints.reserve(POINT_COUNT);
    for (std::size_t i = 0; i < POINT_COUNT; ++i) {
        // map a sinusoid to view
        mPoints << glm::vec2{ 5_dp * float(i), 100_dp * ((glm::sin(i / 10.f) + 1.f) / 2.f) };
    }
}

void DemoGraphView::render() {
    AView::render();

    Render::lines(ASolidBrush{ 0xff0000_rgb }, mPoints);

}
