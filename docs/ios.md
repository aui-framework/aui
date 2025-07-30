@page ios iOS (operating system)
@details
@auisourcemarker
iOS is an operating system developed by Apple, designed for their mobile devices such as iPhone, iPad, and iPod touch.
It provides a secure, user-friendly interface to manage apps, files, music, photos, and other content.

@warning
At the moment, support of iOS in AUI Framework is in early stage and lacks a lot of features.

iOS shares its XNU kernel and codebase with @ref macos.

# Key considerations

- iOS is easier to support than @ref android due to limited number of devices and OS versions
- You'd need a @ref macos machine to build and run an iOS app

# Building an AUI app for iOS

iOS apps are created and built with Xcode projects on macOS machines.

- Use template Xcode project
 
  Since Xcode has no builtin CMake support, the template Xcode project invokes CMake configure and build as an external
  CMake script, setting appropriate `CMAKE_TOOLCHAIN_FILE`. The target passed to @ref aui_app is built as XCframework
  (i.e., as a shared library). @ref aui_app puts headers.