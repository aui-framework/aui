// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Updater/AUpdater.h>
#include <gmock/gmock.h>

#ifdef AUI_ENTRY
#undef AUI_ENTRY
#endif
#define AUI_ENTRY static int fake_entry(const AStringVector& args)

// AUI_DOCS_OUTPUT: doxygen/intermediate/updater.h
// @defgroup updater aui::updater
// @brief Deliver updates on non-centralized distribution methods
// @details
// This module is purposed for devivering updates to your end users on distribution methods that do not support that by
// themselves (i.e., occasional Windows installers, portables for Windows and Linux, macOS app bundles downloaded from
// your website).
//
// `aui.updater` module expects your program to be installed to user's directory (i.e., updating does not require admin
// priveleges). If that's not your case, you'll need to update your installer configuration to install to user's
// directory (i.e., in `AppData`).
//
// `aui.updater` lives inside of entrypoint of your application. It needs you to pass program arguments and exit if
// it returns true.
namespace {
class MainWindow {
public:
    MainWindow(const _<AUpdater>& updater) {}
    void show() {}
};
}

// AUI_DOCS_CODE_BEGIN
AUI_ENTRY {
    auto updater = _new<AUpdater>();
    if (updater->needsExit(args)) {
        return 0;
    }
    // your program routines (i.e., open a window)
    _new<MainWindow>(updater)->show();
    return 0;
}
// AUI_DOCS_CODE_END
//
// You can pass updater instance to your window (as shown in the example) and display update information there and
// perform the update as requested.
//
// # Update process
//
// AUpdater once per some period of time checks for update by itself. Once update is found, it signals that an update
// was found and starts to download and unpack. Once unpacked, AUpdater signals that the update is available for
// installation which basically about replacing files (no network operations will be performed).
//
// @msc
// a[label = "Your App"],
// u[label = "AUpdater", URL = "@ref AUpdater"];
// a -> u [label = "needsExit", URL = "@ref AUpdater::needsExit"];
// a <- u [label = "false"];
//
// --- [label="Normal App Lifecycle"];
// ...;
// a -> u [label = "checkForUpdates", URL = "@ref AUpdater::checkForUpdates"];
// a <- u [label = "false"];
// ...;
//
// --- [label="Update published"];
// a <- u [label = "emit updateAvailable signal", URL = "@ref AUpdater::updateAvailable"];
// u box u [label = "download and unpack update"];
// a <- u [label = "emit updateDownloaded signal", URL = "@ref AUpdater::updateDownloaded"];
// --- [label="Optionally, you can checkForUpdates again"];
// a -> u [label = "checkForUpdates", URL = "@ref AUpdater::checkForUpdates"];
// a <- u [label = "true"];
// --- [label="Your App Prompts User to Update"];
// ...;
// @endmsc
//
// After AUpdater signaled about unpacked update, it waits AUpdater::performUpdate to be called. When
// AUpdater::performUpdate is called (i.e., when user accepted update installation), AUpdater executes a copy of your
// app downloaded before with a special command line argument which is handled by AProcess::needsExit. The copy then
// replaces old application (where it actually installed) with itself (that is, the downloaded, newer copy). After
// operation is complete, it passes the control back to the application. At last, the newly updated application
// performs a cleanup after update.
//
// @msc
// a[label = "Your App"],
// u[label = "AUpdater", URL = "@ref AUpdater"],
// da[label = "Newer Copy of Your App"],
// du[label = "AUpdater in App Copy", URL = "@ref AUpdater"];
// a :> u [label = "performUpdate", URL = "@ref AUpdater::performUpdate"];
// u :> da [label = "Execute with update arg"];
// a box u [label = "Process Finished"];
// da box du [label = "Process Started"];
// da -> du [label = "needsExit", URL = "@ref AUpdater::needsExit"];
// du box du [label = "Replace Old App with Itself"];
// a <: du [label = "Execute"];
// da <- du [label = "true"];
// da box du [label = "Process Finished"];
// a box u [label = "Process Started"];
// a -> u [label = "needsExit", URL = "@ref AUpdater::needsExit"];
// u box u [label = "cleanup download dir"];
// a <- u [label = "false"];
// a box u [label="Normal App Lifecycle"];
// ...;
// @endmsc

TEST(UpdaterTest, ExampleCase) {}
