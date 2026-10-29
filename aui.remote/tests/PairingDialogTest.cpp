#include <AUI/UITest.h>
#include <AUI/View/ATextField.h>
#include <gmock/gmock.h>

#include "ui/window/PairingDialog.h"

class PairingDialogTest : public testing::UITest {};

/**
 * The code field is focused when the dialog is opened, the code can be typed right away.
 */
TEST_F(PairingDialogTest, CodeCanBeTypedRightAway) {
    auto parent = _new<AWindow>();
    parent->show();
    auto dialog = _new<PairingDialog>(*parent, aui_remote::AppInfo { .pid = 1, .exe = "app" });
    dialog->show();
    uitest::frame();

    for (auto c : AString("123456")) {
        dialog->onCharEntered(c);
        uitest::frame();
    }
    By::type<ATextField>().check(text("123456"));
}
