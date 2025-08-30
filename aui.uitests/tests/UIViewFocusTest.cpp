//
// Created by dervisdev on 1/30/2023.
//

//
// Created by dervisdev on 1/30/2023.
//

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>

class UIViewFocus: public testing::UITest {
protected:
    class TestWindow: public AWindow {
    public:
        bool containerFocused = false;
        TestWindow() {
            _<AViewContainer> inner;
            auto container = Centered {
                    inner = Vertical {
                            _new<AButton>("Say hello") AUI_LET { it->setDefault(); }
                    }
            };

            inner.connect(&AView::childFocused, this, [] (_<AView> child) {
                child->setVisibility(Visibility::INVISIBLE);
            });

            inner.connect(&AView::focusAcquired, this, [&] () {
                containerFocused = true;
            });

            setContents(container);
            pack();
        }
    };

    _<TestWindow> mTestWindow;
    void SetUp() override {
        UITest::SetUp();
        mTestWindow = _new<TestWindow>();
        mTestWindow->show();
    }

    void TearDown() override {
        UITest::TearDown();
    }
};


/**
 * Checks that parent receives child focused message
 */
TEST_F(UIViewFocus, ChildFocusedTest) {
    auto buttonMatcher = By::text("Say hello");

    //check visible at first
    buttonMatcher.check(visible(), "Button visible before focus");

    //focus child
    By::text("Say hello").one()->focus();

    //check not visible now
    buttonMatcher.check(notVisible(), "Button invisible after focus");
}

/**
 * Checks parent isn't focused when child is
 */
TEST_F(UIViewFocus, ParentNotFocusingTest) {
    auto buttonMatcher = By::text("Say hello");
    auto containerMatcher = By::type<AViewContainer>();

    //check flag false
    EXPECT_FALSE(mTestWindow->containerFocused);

    //focus child
    buttonMatcher.perform(click());

    //still false
    EXPECT_FALSE(mTestWindow->containerFocused);
}