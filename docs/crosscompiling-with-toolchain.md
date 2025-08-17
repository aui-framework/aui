# Replacing AUI mobile project

!!! note

    This page describes an advanced scenario of integrating AUI. For basic usage, see [crosscompiling.md]

Some projects may require their custom mobile project (Android Studio or Xcode), for example to specific mobile
features (such as Google Play Games integration, Face ID, etc). In this case, you would have to create your own project
and link AUI as a library.

You will need a toolchain file which describes your target platform.

In Android, a Gradle-driven project provides the toolchain for you. You don't even need to specify CMAKE_TOOLCHAIN_FILE,
Gradle does it for you.

In iOS, there's no "official" toolchain so there are community-provided toolchains
(like the [one used in AUI](https://raw.githubusercontent.com/aui-framework/aui/master/cmake/toolchains/arm64-ios.cmake)).

AUI.Boot forwards `CMAKE_TOOLCHAIN_FILE` across all the dependencies.

## Android

1. Create new Android Studio project with native C++ support.
2. In your `app/src/cpp/CMakeLists.txt`, `add_subdirectory` your C++ application or directly `auib_import` the AUI
   framework (unrecommended because you will experience difficulties when compiling for other platforms).
3. Copy & paste AUI's [native Java code](https://github.com/aui-framework/aui/tree/master/platform/android) for Android
   to your project. You may also want to adjust your `app.gradle` and `AndroidManifest.xml`.
4. To draw AUI, you should use `GLSurfaceView` and [pass rendering routines](https://github.com/aui-framework/aui/blob/master/platform/android/lib/src/java/com/github/aui/android/AuiView.kt)
   to AUI. Also, take care of handling user input. Also, you can inflate [AuiView](https://github.com/aui-framework/aui/blob/master/platform/android/lib/src/java/com/github/aui/android/AuiView.kt)
   to your own Android layout.

## iOS
1. Create new Xcode project.
2. Since Xcode does not support CMake, you will have to compile AUI (and your C++ AUI application) by yourself (by
   creating Xcode rules or manually dropping compiled binaries to Xcode project).
3. [Take a look](https://github.com/aui-framework/aui/blob/master/aui.views/src/AUI/Platform/ios/AUIViewController.mm) on how does AUI handles its routines on iOS and implement them by yourself in your project.
