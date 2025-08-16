# Build-time macros

!!! note

    This page is about macros generated in build-time (i.e., platform specifics). For macros defined in C++ code, see
    [useful_macros].

## Writing platform dependent code

AUI provides a set of `AUI_PLATFORM_*` and `AUI_COMPILER_*` definitions for platform and compiler checking to use in
both CMake and C++.

### Platform checks

<table markdown>
   <tr>
     <th>Platform</th>
     <th>C++</th>
     <th>CMake</th>
     <th>Platform specific dir(s)</th>
   </tr>
   <tr>
     <td markdown>
       [windows]
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_WIN
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_WIN)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/win32
       src/platform/win32
     </td>
   </tr>

   <tr>
     <td markdown>
       [linux]
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_LINUX
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_LINUX)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/linux
       src/platform/linux
     </td>
   </tr>

   <tr>
     <td markdown>
       [macos]
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_MACOS
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_MACOS)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/macos
       src/platform/macos
     </td>
   </tr>

   <tr>
     <td markdown>
       [android]
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_ANDROID
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_ANDROID)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/android
       src/platform/android
     </td>
   </tr>

   <tr>
     <td markdown>
       [ios] (both iPhone and iPad)
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_IOS
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_IOS)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/ios
       src/platform/ios
     </td>
   </tr>

   <tr>
     <td markdown>
       Apple([macOS](macos), [iOS](ios))
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_APPLE
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_APPLE)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/apple
       src/platform/apple
     </td>
   </tr>

   <tr>
     <td markdown>
       Unix([Linux](linux), [Android](android), [macOS](macos), [iOS](ios))
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_UNIX
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_UNIX)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/unix
       src/platform/unix
     </td>
   </tr>

   <tr>
     <td markdown>
       [emscripten]
     </td>
     <td markdown>
       ```cpp
        #if AUI_PLATFORM_EMSCRIPTEN
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_PLATFORM_EMSCRIPTEN)
          # ...
        endif()
       ```
     </td>
     <td markdown>
       src/Platform/emscripten
       src/platform/emscripten
     </td>
   </tr>
</table>

### Platform specific sources

With AUI, the platform dependent code can be placed in `src/Platform/<PLATFORM_NAME>` or `src/platform/<PLATFORM_NAME>`
dirs, where `<PLATFORM_NAME>` is one of the supported platforms (see the table above).

!!! note

    Both cases (Platform and platform) were added to honor projects with various directory naming agreements.


### Compiler checks

<table>
   <tr>
     <th>Compiler</th>
     <th>C++</th>
     <th>CMake</th>
   </tr>
   <tr>
     <td markdown>
       MSVC
     </td>
     <td markdown>
       ```cpp
        #if AUI_COMPILER_MSVC
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_COMPILER_MSVC)
          # ...
        endif()
       ```
     </td>
   </tr>

   <tr>
     <td markdown>
       GCC (including MinGW)
     </td>
     <td markdown>
       ```cpp
        #if AUI_COMPILER_GCC
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_COMPILER_GCC)
          # ...
        endif()
       ```
     </td>
   </tr>

   <tr>
     <td markdown>
       CLANG
     </td>
     <td markdown>
       ```cpp
        #if AUI_COMPILER_CLANG
          // ...
        #endif
       ```
     </td>
     <td markdown>
       ```cmake
        if(AUI_COMPILER_CLANG)
          # ...
        endif()
       ```
     </td>
   </tr>
</table>

## Other

### AUI_MODULE_NAME

Target name exposed by [aui_module](aui_module.md) and [aui_executable](aui_executable.md).

### AUI_CMAKE_PROJECT_VERSION

`${CMAKE_PROJECT_VERSION}` exposed by [aui_module] and [aui_executable].

`CMAKE_PROJECT_VERSION` is typically defined by [project](https://cmake.org/cmake/help/latest/command/project.html) 
CMake command:

<!-- aui:snippet test/minimal_deployment_test/CMakeLists.txt AUI_CMAKE_PROJECT_VERSION -->

### API_\<module name\>

`dllexport` (on Windows) or `visibility` (on other platforms) policy for the symbol.

```cpp
class API_AUI_VIEWS AView ... { // defined in aui.views module
  ...
};
```