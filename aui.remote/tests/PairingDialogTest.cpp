#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATextField.h>
#include <gmock/gmock.h>

#include "ui/window/PairingDialog.h"

using namespace declarative;

class PairingDialogTest : public testing::UITest {};

/**
 * The code field is focused when the dialog is opened, the code can be typed right away.
 */
TEST_F(PairingDialogTest, CodeCanBeTypedRightAway) {
    auto parent = _new<AWindow>();
    parent->show();
    auto dialog = _new<AWindow>();
    dialog->setContents(pairingDialog(aui_remote::AppInfo { .pid = 1, .exe = "app" }, [] {}));
    dialog->show();
    uitest::frame();

    for (auto c : AString("123456")) {
        dialog->onCharEntered(c);
        uitest::frame();
    }
    By::type<ATextField>().check(text("123456"));
}

/**
 * The contents are given to AWindow::setContents, which takes the views of the container and drops the container itself,
 * and still can close the window.
 */
TEST_F(PairingDialogTest, CancelClosesTheWindow) {
    auto dialog = _new<AWindow>();
    dialog->setContents(pairingDialog(aui_remote::AppInfo { .pid = 1, .exe = "app" }, [] {}));
    dialog->show();
    uitest::frame();
    EXPECT_FALSE(dialog->isClosed());

    By::text("Cancel").perform(click());
    uitest::frame();
    EXPECT_TRUE(dialog->isClosed());
}
