# Adding auto-update to an app

This page walks through connecting [AUpdater] to an application. For the update process itself (what happens between
`checkForUpdates` and the restart), see [updater]. To get releases that the updater can consume, see
[updater-publishing].

## Link the module

Add `updater` to the components of `auib_import` and link `aui::updater`. The module uses `aui::curl`, `aui::json` and
`aui::crypt` internally, so list them as well.

```cmake
auib_import(aui https://github.com/aui-framework/aui
        COMPONENTS core views curl json crypt updater
        VERSION ${AUI_VERSION})

aui_executable(${PROJECT_NAME})
aui_link(${PROJECT_NAME} PRIVATE aui::core aui::views aui::curl aui::json aui::updater)
```

[app-template] has this already set up, including GitHub Actions that publish releases.

## Subclass AUpdater

[AUpdater] is a strategy: it handles the mechanics of unpacking and deploying, while you decide where updates come from
by implementing `checkForUpdatesImpl` and `downloadUpdateImpl`. Both return an `AFuture<void>` and run on whatever
thread you choose (typically a thread pool). A complete GitHub-based implementation is shown in [updater-publishing].

## Call handleStartup first

`handleStartup` must be called as early as possible, before any window is shown. It processes the `--aui-updater-*`
arguments passed by a previous instance of your app and, when an update is being deployed, terminates the process with
`std::exit(0)`.

<!-- aui:snippet aui.updater/tests/UpdaterTest.cpp updater_entry -->

Keep the updater instance alive for the lifetime of the application and pass it to the UI, see [updater-ui].

## Check that the updater is available

`AUpdater::isAvailable()` returns false when the executable is not writable by the current user (i.e., installed to
`Program Files`, or a system package), or the platform is unsupported. In that case `AUpdater` turns into a stub and
sets its status to `AUpdater::StatusNotAvailable`. Hide update controls in that state instead of letting the user press
buttons that do nothing:

```cpp
if (!AUpdater::isAvailable()) {
    // do not show "Check for updates" button
}
```

## Version of your app

The versions are compared with [aui::updater::Semver]. For your own version, use the one from `project(... VERSION x.y.z)`
in your root `CMakeLists.txt`: AUI passes it to every `aui_executable` and `aui_module` as the
`AUI_CMAKE_PROJECT_VERSION` macro.

<!-- aui:snippet aui.updater/tests/VersionTest.cpp semver_parse -->

Only the three numeric components are compared; suffixes such as `-beta` are dropped, so `1.2.3-beta` and `1.2.3` are
the same version for the updater.

## Supported platforms

Windows (portable ZIP, or an installer that installs into the user's directory) and Linux (portable ZIP) are supported.
On macOS and mobile platforms the stores or your own distribution mechanism take care of updates. See [updater] for
the full table.
