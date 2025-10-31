#include "MainWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/Platform/APlatform.h>
#include <AUI/View/ADrawableView.h>

using namespace declarative;

MainWindow::MainWindow(): AWindow("Project template app", 300_dp, 200_dp) {
    setContents(
        Centered{
            Vertical{
                Centered { Icon { ":img/logo.svg" } with_style { FixedSize(64_dp) } },
                Centered { Label { "Hello world from AUI!" } },
                _new<AButton>("Visit GitHub repo").connect(&AView::clicked, this, [] {
                    APlatform::openUrl("https://github.com/aui-framework/aui");
                }),
                _new<AButton>("Visit docs").connect(&AView::clicked, this, [] {
                    APlatform::openUrl("https://aui-framework.github.io/");
                }),
                _new<AButton>("Submit an issue").connect(&AView::clicked, this, [] {
                    APlatform::openUrl("https://github.com/aui-framework/aui/issues/new");
                }),
            }
        }
    );
}
