# App Build Overview

This page describes how an AUI application is built.

## Trivia

On this page, libraries and executables are represented with [windows] file name extensions with shared linking.
Other platforms and configurations follow the same principles but with different file types.

- LIBRARY.dll refers to a binary (compiled) representation of LIBRARY
- PROGRAM.exe refers to a binary (compiled) representation of PROGRAM (executable)

Also, on this page, the build system is demonstrated on [minimal-ui-template-with-assets](minimal_ui_assets.md).

## Generic scenario { #BUILD_SCENARIO_GENERIC }

The most common and straightforward scenario. CMake handles everything.

This is applicable for most desktop platforms.

``` mermaid
flowchart BT
    subgraph ide["User's IDE (optional)"]
        CLion
        VSCode
    end

    subgraph cmake_cli[CMake CLI]
        cmake_configure["mkdir build && cd build && cmake .."]
        cmake_build["cmake --build . -t app"]
    end

    cmake_cli --> ide

    auib["aui.boot.cmake"] -->|download aui.boot| cmake_configure
    auib_import["auib_import(aui aui-framework/aui)"] -->|import aui| cmake_configure
    auib ---> auib_import

    subgraph AUI
        aui.core.dll
        aui.views.dll
        aui_other["..."]
        aui.toolbox.exe
    end

    subgraph sys["System Libraries"]
        direction LR
        c++["C++ STD Lib"]
        sys_other[...]
    end

    subgraph aui_deps["AUI Dependencies"]
        direction LR
        ZLIB
        OpenSSL
        deps[...]
    end
    sys -->|find_package| AUI
    aui_deps -->|aui.boot.cmake| AUI

    AUI ---> auib_import
    AUI ---> app.exe
    app.exe --> cmake_build
    main.cpp -->|C++ compiler| app.exe
    icon.svg.cpp -->|C++ compiler| app.exe
    ass_conv[convert icon.svg to icon.svg.cpp] --> icon.svg.cpp
    aui.toolbox.exe ---> ass_conv
    assets/icon.svg --> ass_conv
```

Things to note:

- [aui.boot] is downloaded in your `CMakeLists.txt`, which downloads AUI itself and all its dependencies.
- Among with AUI libraries, there's `aui.toolbox`, which participates in the build process. In particular, it converts
  [aui-assets] to cpp files, compressing them, making them available in the application, effectively embedding them to
  the application binary.

## Mobile scenario

On mobile platforms, the build process is overcomplicated by:

- Need to support multiple architectures (ARM, x86, etc)
- Platform specific packaging requirements (APK, IPA)
- Integrating with platform-specific build systems (Gradle, Xcode)
- Interfacing to Kotlin/Swift (Java/Objective-C) from/to C++
- Deliver Kotlin/Swift (Java/Objective-C) platform-specific code through CMake

In this section, we'll consider Android (armv7 + arm64) as the target platform and Windows as the build host platform.
On combinations of the other host/target platforms, the build process remains similar.

```mermaid
flowchart BT
  gradle_gen --> gradle_build
  app.apk --> gradle_build

  gradle_build --> as["Android Studio"]

%% app.apk
  app.apk

  AUI --->|Template| gradle_gen

  arm64/libapp.so -->|CMake| app.apk
  armv7/libapp.so -->|CMake| app.apk

  AUI_arm64 --------->|aui.boot.cmake| arm64/libapp.so
  AUI_armv7 --------->|aui.boot.cmake| armv7/libapp.so

%% HOST
  AUI ---> auib_import
  subgraph cmake_cli[CMake CLI]
    cmake_configure["mkdir build && cd build && cmake .. -DAUI_BUILD_FOR=android"]
    cmake_build["cmake --build . -t apps"]
  end

  auib["aui.boot.cmake"] -->|download aui.boot| cmake_configure
  auib_import["auib_import(aui aui-framework/aui)"] -->|import aui| cmake_configure
  auib --> auib_import

  subgraph AUI["AUI (Host)"]
    direction LR
    aui.core.dll
    aui.views.dll
    aui.toolbox.exe
    gradle_project_template["Android project template"]
  end

  subgraph sys["System Libraries (Host)"]
    direction LR
    c++["C++ STD Lib"]
    sys_other[...]
  end

  subgraph aui_deps["AUI Dependencies (Host)"]
    direction LR
    ZLIB
    OpenSSL
    deps[...]
  end
  sys -->|find_package| AUI
  aui_deps --->|aui.boot.cmake| AUI

%% arm64
  subgraph AUI_arm64["AUI (arm64)"]
    direction LR
    libaui.core.a_arm64["libaui.core.a"]
    libaui.views.a_arm64["libaui.views.a"]
    libaui_other_arm64["..."]
  end

  subgraph sys_arm64["System Libraries (arm64)"]
    direction LR
    c++_arm64["C++ STD Lib"]
    sys_other_arm64[...]
  end

  subgraph aui_deps_arm64["AUI Dependencies (arm64)"]
    direction LR
    ZLIB_arm64[ZLIB]
    OpenSSL_arm64[OpenSSL]
    deps_arm64[...]
  end
  sys_arm64 -->|find_package| AUI_arm64
  aui_deps_arm64 --->|aui.boot.cmake| AUI_arm64
  icon.svg.cpp -->|"C++ compiler (arm64)"| arm64/libapp.so
  main.cpp -->|"C++ compiler (arm64)"| arm64/libapp.so

%% armv7
  subgraph AUI_armv7["AUI (armv7)"]
    direction LR
    libaui.core.a_armv7["libaui.core.a"]
    libaui.views.a_armv7["libaui.views.a"]
    libaui_other_armv7["..."]
  end

  subgraph sys_armv7["System Libraries (armv7)"]
    direction LR
    c++_armv7["C++ STD Lib"]
    sys_other_armv7[...]
  end

  subgraph aui_deps_armv7["AUI Dependencies (armv7)"]
    direction LR
    ZLIB_armv7[ZLIB]
    OpenSSL_armv7[OpenSSL]
    deps_armv7[...]
  end
  sys_armv7 -->|find_package| AUI_armv7
  aui_deps_armv7 --->|aui.boot.cmake| AUI_armv7
  main.cpp -->|"C++ compiler (armv7)"| armv7/libapp.so
  icon.svg.cpp ---->|"C++ compiler (armv7)"| armv7/libapp.so

  ass_conv[convert icon.svg to icon.svg.cpp] --> icon.svg.cpp
  assets/icon.svg --> ass_conv
  AUI ----->|aui.toolbox.exe | ass_conv

  gradle_gen["Generate Gradle Project"] --> cmake_build
  gradle_build["./gradle build"] --> cmake_build
```

Things to note:

- Mobile scenario slightly resembles [BUILD_SCENARIO_GENERIC].
- There are multiple CMake processes: one of the host, which defines `apps` meta target, which invokes Gradle, which
  invokes another CMakes per architecture, supplying `CMAKE_TOOLCHAIN`.
- There are several AUI instances: for the host platform, arm64 (for the target) and armv7 (for the target).
- The host version of AUI supplies `aui.toolbox`, which is used to compile assets; and a copy of Gradle/Xcode project
  to generate from. Also, this template delivers Kotlin/Swift platform code of AUI.
- Libraries (the components defined with [aui_module]) are built statically. This is needed to avoid code signing
  hell on iOS.
- On Android, the application executable (the one defined with [aui_executable]) is compiled as a shared library,
  because JVM is the executable of an Android application, which is then loads a shared library containing native code.