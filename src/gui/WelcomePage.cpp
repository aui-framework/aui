#include "WelcomePage.h"

#include <AUI/Platform/AClipboard.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/View/ASpinnerV2.h>

#include "remote/KeyStore.h"
#include "remote/Skill.h"

using namespace declarative;
using namespace aui_remote;

WelcomePage::WelcomePage() {
    mSkillInstalled = skill::isInstalled();
    mSkillStatus = "Installs {}"_format(skill::defaultFile());

    setContents(Centered {
      Vertical {
        Vertical {
          CheckBox {
            .checked = AUI_REACT(*mSkillInstalled),
            .onCheckedChange = [this](bool wanted) { skillToggled(wanted); },
            .content = Label { "Let Claude drive AUI development" },
          },
          Label { AUI_REACT(*mSkillStatus) } AUI_OVERRIDE_STYLE { TextColor { 0xff777777_argb }, Margin { 0, 0, 0, 20_dp } },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp } },
        Label { "Available connections" } AUI_OVERRIDE_STYLE { FontSize { 12_pt } },
        AUI_DECLARATIVE_FOR(row, *mRows, AVerticalLayout) {
            return connectionRow(row);
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
              .content = Label { "Copy" },
              .onClick = [] { AClipboard::copyToClipboard("ARemoteDebug::start();"); },
            },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
          Label { "and it will show up here." },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } } AUI_LET {
            AObject::connect(AUI_REACT(mRows->empty() ? Visibility::VISIBLE : Visibility::GONE), it->visibility());
        },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp }, MinSize { 480_dp, 0 } },
    });
}

void WelcomePage::setApps(const AVector<AppInfo>& apps, const AVector<std::uint32_t>& openPids) {
    AVector<Row> rows;
    KeyStore store;
    for (const auto& app : apps) {
        Action action = store.load(app).hasValue() ? Action::CONNECT : Action::PAIR;
        if (openPids.contains(app.pid)) {
            action = Action::OPEN;
        }
        rows << Row { app, action };
    }
    if (*mRows != rows) {
        mRows = std::move(rows);
    }
}

_<AView> WelcomePage::connectionRow(const Row& row) {
    AString text = "Open";
    switch (row.action) {
        case Action::PAIR: text = "Pair..."; break;
        case Action::CONNECT: text = "Connect..."; break;
        case Action::OPEN: break;
    }
    return Horizontal {
      Label { "Local" } AUI_OVERRIDE_STYLE { MinSize { 70_dp, 0 } },
      Label { row.app.displayName() },
      SpacerExpanding {},
      Button {
        .content = Label { std::move(text) },
        .onClick = [this, app = row.app, action = row.action] {
            if (action == Action::PAIR) {
                emit pairRequested(app);
            } else {
                emit openRequested(app);
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

void WelcomePage::skillToggled(bool wanted) {
    mSkillInstalled = wanted;
    try {
        if (wanted) {
            skill::install(skill::builtinTemplate(), AProcess::self()->getPathToExecutable());
        } else {
            skill::uninstall();
        }
        mSkillStatus = "Installs {}"_format(skill::defaultFile());
    } catch (const AException& e) {
        mSkillStatus = "Can't change the skill: " + e.getMessage();
    }
    // brings the check box back if the change has failed
    mSkillInstalled = skill::isInstalled();
}
