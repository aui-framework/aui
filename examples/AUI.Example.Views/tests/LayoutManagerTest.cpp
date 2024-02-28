// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <ExampleWindow.h>
#include <AUI/View/ATextField.h>

/**
 * <p>
 *   A bug appeared in this example application related to AUI layout managers. This suite checks the following issues:
 * </p>
 * <ul>
 *   <li>height of the latest element sometimes (not always!) seems to be incorrect</li>
 *   <li>latest elements sometimes (again, not always!) seems to be not fit to the parent's border</li>
 * </ul>
 *
 * <p>Details: <a href="https://github.com/aui-framework/aui/issues/18">https://github.com/aui-framework/aui/issues/18</a></p>
 * <p>Photo:</p>
 * <img src="https://user-images.githubusercontent.com/19491414/144756936-173e5d65-9433-4c00-92d6-142ca217164d.png" />
 *
 */


class UILayoutManager: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();
        _new<ExampleWindow>()->show();
    }

    void TearDown() override {
        UITest::TearDown();
    }
};


/**
 * Checks height of all buttons - they should be the same (see "Show all views..." button above).
 */
TEST_F(UILayoutManager, Height) {
    // check height
    (By::text("Common button") | By::text("Show all views...")).check(sameHeight(), "height mismatch");
}


/**
 * Checks alignment (see "Alex2772, 2021, alex2772.ru" - it is not perfectly aligned)
 */
TEST_F(UILayoutManager, LastElementAlignment) {
    // copyright width can be also not minimal
    By::name("#copyright").check(widthIsMinimal(), "copyright width should be minimal");
}

/**
 * Checks alignment (looks like buttons and list views are not perfectly aligned)
 */
TEST_F(UILayoutManager, ButtonsAlignment) {
    // buttons column should be perfectly aligned
    By::name("Common button")
        .parent()
        .allChildren()
        .check(leftRightAligned(), "elements should be perfectly aligned");
}
