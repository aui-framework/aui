# aui_app

Provide application info for deployment (i.e. name, author, license, icon, etc...).

`aui_app` unifies the packaging process across all platforms. For (mostly) all desktop platforms, it acts in
collaboration with [CPack](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html).

- `ZIP`, `TGZ`
- `DEB`
- `NSIS`
- `WIX`
- `External`

For the targets that require [cross-compilation](crosscompiling.md), it requires `[AUI_BUILD_FOR]` to be
set, and, in fact, does not involve CPack in such case.

For all packaging methods, `aui_app` assumes it is called once per CMake project. It is relevant for packaging methods
that expect one "entrypoint" per package.
- `aui_app` -> Android APK
- `aui_app` -> iOS app bundle
- `aui_app` -> Linux Flatpak
- `aui_app` -> Linux AppImage

For this reason, `aui_app` populates some empty `CPACK_*` variables. This approach allows you not to bother about
various installation methods but also override the variables if needed.

For more info about packaging methods, see [aui-configure-flags].

## Syntax

<!-- aui:snippet examples/ui/views/CMakeLists.txt aui_app -->

```cmake
aui_app(
    # common
    <TARGET <target-name>>
    <NAME <application-name>>
    [ICON <path-to-icon-svg>]
    [VENDOR <vendor-name>]
    [COPYRIGHT <copyright-string>]
    [VERSION <version>]
        
    [NO_INCLUDE_CPACK]

    # android only
    [ANDROID_PACKAGE <android-app-package>]
        
    # linux only
    [LINUX_DESKTOP <path-to-desktop-file>]
    
    # apple only
    [APPLE_TEAM_ID <apple-team-id>]
    [APPLE_BUNDLE_IDENTIFIER <bundle-identifier>]
    [APPLE_SIGN_IDENTITY <sign-identity>]

    # ios only
    [IOS_VERSION <target-ios-version>]
    [IOS_DEVICE <target-ios-device>]
    [IOS_CONTROLLER <custom-ios-controller-class-name>]
)
```

### TARGET

Specify main executable of the application which will be executed when your application is launched.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|+||
|Linux|+||
|macOS|+||
|Android|+||
|iOS|+||

### NAME

Specify application display name that would appear in system's UIs (i.e., start menu, launchpad, desktop, control panel).

This value is availabile in C++ via `app_info` namespace:

```cpp
#include <AUI/AppInfo.h>
int main() { auto appName = aui::app_info::name; }
```

|Platform|Required|Traits|
|--------|--------|------|
|Windows|+||
|Linux|+||
|macOS|+||
|Android|+||
|iOS|+||

{{% cmake_list_all_vars_affected_by('cmake/aui.build.cmake', 'APP_NAME', 'APP_NAME') %}}

### ICON { #aui_app_ICON }

Specify SVG icon location (defaults to `icon.svg` if exists).

See [app-icon] for best practices.

| Platform | Required | Traits                                                |
|----------|----------|-------------------------------------------------------|
| Windows  |          | generates `*.ico` and `*.bmp` files from your `svg`   |
| Linux    | +        | generates Freedesktop-compliant icons from your `svg` |
| macOS    | +        | generates `*.icns` file from your `svg`               |
| Android  | +        | generates icon assets from your `svg`                 |
| iOS      | +        | generates `*.icns` file from your `svg`               |

Using `aui.toolbox`, `aui_app` generates image assets from your icon depending on the [app packaging method](packaging.md).
chosen.

{{% cmake_list_all_vars_affected_by('cmake/aui.build.cmake', '(APP_ICON|_ico)', 'APP_ICON') %}}

### VENDOR

Specify application author's name or company.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|||
|Linux|||
|macOS|||
|Android|||
|iOS|||

{{% cmake_list_all_vars_affected_by('cmake/aui.build.cmake', 'APP_VENDOR', 'APP_VENDOR') %}}

### COPYRIGHT

Specify application copyright string (not file)

|Platform|Required|Traits|
|--------|--------|------|
|Windows|||
|Linux||does not matter|
|macOS|+||
|Android|||
|iOS|+||

{{% cmake_list_all_vars_affected_by('cmake/aui.build.cmake', 'APP_COPYRIGHT', 'APP_COPYRIGHT') %}}


### VERSION

Specify application version. If not set, the value of `PROJECT_VERSION` used instead.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|+||
|Linux|+||
|macOS|+||
|Android|+||
|iOS|+||

{{% cmake_list_all_vars_affected_by('cmake/aui.build.cmake', 'APP_VERSION', 'APP_VERSION') %}}

### NO_INCLUDE_CPACK

By default `aui_app` includes CPack in order to allow to generate packages using CPack. You can disable this behaviour by specifying `NO_INCLUDE_CPACK` flag.

### ANDROID_PACKAGE

Android app package name.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|||
|Linux|||
|macOS|||
|Android|+||
|iOS|||

### LINUX_DESKTOP

Specify custom `*.desktop` file. If not set, `*.desktop` file is generated from `aui_app` arguments.

|Platform|Required|Traits|
|--------|--------|------|
|Linux|+||


### APPLE_TEAM_ID

Specify your Apple Team ID (https://discussions.apple.com/thread/7942941).

|Platform|Required|Traits|
|--------|--------|------|
|macOS|+||
|iOS|+||

### APPLE_BUNDLE_IDENTIFIER

Specify bundle identifier. If not set, the value of `NAME` used instead.

|Platform|Required|Traits|
|--------|--------|------|
|macOS|||
|iOS|||

### APPLE_SIGN_IDENTITY

Specify Apple sign identity. Defaults to "iPhone Developer".

|Platform|Required|Traits|
|--------|--------|------|
|macOS||unused|
|iOS|||

### IOS_VERSION

Specify target iOS platform. Defaults to "14.3".

|Platform|Required|Traits|
|--------|--------|------|
|iOS|||

### IOS_DEVICE

Specify target iOS device. Applicable values are `IPHONE`, `IPAD` or `BOTH`. Defaults to `BOTH`.

|Platform|Required|Traits|
|--------|--------|------|
|iOS|||

### IOS_CONTROLLER

Specify custom view controller class name. Defaults to `AUIViewController`.

|Platform|Required|Traits|
|--------|--------|------|
|iOS|||
