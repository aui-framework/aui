#include "WelcomePage.h"

#include <AUI/Common/AProperty.h>
#include <AUI/Platform/AClipboard.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/View/ASpinnerV2.h>

#include "network/Skill.h"

using namespace declarative;
using namespace aui_remote;

namespace {
_<AView> heading(AString text) { return Label { std::move(text) } AUI_OVERRIDE_STYLE { FontSize { 12_pt } }; }

struct State {
    AProperty<bool> skillInstalled = skill::isInstalled();
    AProperty<AString> skillStatus = "Installs {}"_format(skill::defaultFile());
    std::function<void(AppInfo)> onPair;
    std::function<void(AppInfo)> onOpen;

    void skillToggled(bool wanted) {
        skillInstalled = wanted;
        try {
            if (wanted) {
                skill::install(skill::builtinTemplate(), AProcess::self()->getPathToExecutable());
            } else {
                skill::uninstall();
            }
            skillStatus = "Installs {}"_format(skill::defaultFile());
        } catch (const AException& e) {
            skillStatus = "Can't change the skill: " + e.getMessage();
        }
        // brings the check box back if the change has failed
        skillInstalled = skill::isInstalled();
    }

    _<AView> connectionRow(const ConnectionRow& row) {
        AString text = "Open";
        switch (row.action) {
            case ConnectionRow::Action::PAIR: text = "Pair..."; break;
            case ConnectionRow::Action::CONNECT: text = "Connect..."; break;
            case ConnectionRow::Action::OPEN: break;
        }
        return Horizontal {
          Label { "Local" } AUI_OVERRIDE_STYLE { MinSize { 70_dp, 0 } },
          Label { row.app.displayName() },
          SpacerExpanding {},
          Button {
            Label { std::move(text) },
            [this, app = row.app, action = row.action] {
                if (action == ConnectionRow::Action::PAIR) {
                    onPair(app);
                } else {
                    onOpen(app);
                }
            },
          },
        } AUI_OVERRIDE_STYLE {
          LayoutSpacing { 8_dp },
          Padding { 6_dp, 10_dp },
          BackgroundSolid { 0x14000000_argb },
          BorderRadius { 4_dp },
        };
    }
};
}   // namespace

_<AView> welcomePage(_<AProperty<AVector<ConnectionRow>>> rows, std::function<void(AppInfo)> onPair,
                     std::function<void(AppInfo)> onOpen) {
    auto state = _new<State>();
    state->onPair = std::move(onPair);
    state->onOpen = std::move(onOpen);

    return Centered {
      Vertical {
        Vertical {
          CheckBox {
            AUI_REACT(*state->skillInstalled),
            [state](bool wanted) { state->skillToggled(wanted); },
            Label { "Allow Claude to use AUI Remote" },
          },
          Label { AUI_REACT(*state->skillStatus) } AUI_OVERRIDE_STYLE { TextColor { 0xff777777_argb }, Margin { 0, 0, 0, 20_dp } },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp } },
        heading("Available connections"),
        AUI_DECLARATIVE_FOR(row, **rows, AVerticalLayout) {
            return state->connectionRow(row);
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
        Vertical {
          Horizontal {
            _new<ASpinnerV2>(),
            Label { "Looking for applications..." },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
          Label { "Run an application that calls" },
          Horizontal {
            Label { "ARemoteDebug::start();" } AUI_OVERRIDE_STYLE { BackgroundSolid { 0x14000000_argb }, Padding { 4_dp } },
            Button {
              Label { "Copy" },
              [] { AClipboard::copyToClipboard("ARemoteDebug::start();"); },
            },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
          Label { "and it will show up here." },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } } AUI_LET {
            AObject::connect(AUI_REACT((*rows)->empty() ? Visibility::VISIBLE : Visibility::GONE), it->visibility());
        },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, MinSize { 480_dp, 0 } },
    };
}
