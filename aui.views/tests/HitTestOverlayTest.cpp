#include <gtest/gtest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ARulerArea.h>
#include <AUI/View/AViewContainer.h>

using namespace declarative;

namespace {
class Target : public AView {};
}   // namespace

// ARulerArea lays out the wrapped view by itself; if it didn't, the view would not consume clicks and the hit test
// would stop at a (transparent) view stacked above the ruler area.
TEST(ARulerArea, WrappedViewIsHitThroughExpandingOverlay) {
    auto target = _new<Target>();
    target->setFixedSize({ 100, 100 });
    _<AViewContainer> root = Stacked::Expanding {
        _new<ARulerArea>(target),
        Vertical::Expanding {
          Horizontal { SpacerExpanding {}, _new<AButton>("M") },
        },
    }();
    root->layout(0, 0, 600, 400);
    EXPECT_EQ(root->getViewAtRecursive({ 300, 200 }), target);
}
