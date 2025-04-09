@note
This page is about macros generated in build-time (i.e., platform specifics). For macros defined in C++ code, see
@ref useful_macros.

# Writing platform dependent code

AUI provides a set of `AUI_PLATFORM_*` and `AUI_COMPILER_*` definitions for platform and compiler checking to use in
both CMake and C++.

## Platform checks

<table>
   <tr>
     <th>Platform</th>
     <th>C++</th>
     <th>CMake</th>
     <th>Platform specific dir(s)</th>
   </tr>
   <tr>
     <td>
       @ref windows
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_WIN
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_WIN)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/win32`<br/>
       `src/platform/win32`
     </td>
   </tr>

   <tr>
     <td>
       @ref linux
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_LINUX
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_LINUX)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/linux`<br/>
       `src/platform/linux`
     </td>
   </tr>

   <tr>
     <td>
       @ref macos
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_MACOS
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_MACOS)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/macos`<br/>
       `src/platform/macos`
     </td>
   </tr>

   <tr>
     <td>
       @ref android
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_ANDROID
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_ANDROID)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/android`<br/>
       `src/platform/android`
     </td>
   </tr>

   <tr>
     <td>
       @ref ios <br/>(both iPhone and iPad)
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_IOS
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_IOS)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/ios`<br/>
       `src/platform/ios`
     </td>
   </tr>

   <tr>
     <td>
       Apple<br/>(@ref macos "macOS", @ref ios "iOS")
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_APPLE
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_APPLE)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/apple`<br/>
       `src/platform/apple`
     </td>
   </tr>

   <tr>
     <td>
       Unix<br/>(@ref linux "Linux", @ref android "Android", @ref macos "macOS", @ref ios "iOS")
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_UNIX
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_UNIX)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/unix`<br/>
       `src/platform/unix`
     </td>
   </tr>

   <tr>
     <td>
       @ref emscripten
     </td>
     <td>
       @code{cpp}
        #if AUI_PLATFORM_EMSCRIPTEN
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_PLATFORM_EMSCRIPTEN)
          # ...
        endif()
       @endcode
     </td>
     <td>
       `src/Platform/emscripten`<br/>
       `src/platform/emscripten`
     </td>
   </tr>
</table>

### Platform specific sources

With AUI, the platform dependent code can be placed in `src/Platform/<PLATFORM_NAME>` or `src/platform/<PLATFORM_NAME>`
dirs, where `<PLATFORM_NAME>` is one of the supported platforms (see the table above).

@note
Both cases (Platform and platform) were added to honor projects with various directory naming agreements.


## Compiler checks

<table>
   <tr>
     <th>Compiler</th>
     <th>C++</th>
     <th>CMake</th>
   </tr>
   <tr>
     <td>
       MSVC
     </td>
     <td>
       @code{cpp}
        #if AUI_COMPILER_MSVC
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_COMPILER_MSVC)
          # ...
        endif()
       @endcode
     </td>
   </tr>

   <tr>
     <td>
       GCC (including MinGW)
     </td>
     <td>
       @code{cpp}
        #if AUI_COMPILER_GCC
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_COMPILER_GCC)
          # ...
        endif()
       @endcode
     </td>
   </tr>

   <tr>
     <td>
       CLANG
     </td>
     <td>
       @code{cpp}
        #if AUI_COMPILER_CLANG
          // ...
        #endif
       @endcode
     </td>
     <td>
       @code{cmake}
        if(AUI_COMPILER_CLANG)
          # ...
        endif()
       @endcode
     </td>
   </tr>
</table>

# Other

## AUI_MODULE_NAME

Target name exposed by [aui_module](@ref docs/aui_module.md) and [aui_executable](@ref docs/aui_executable.md).

## AUI_CMAKE_PROJECT_VERSION

`${CMAKE_PROJECT_VERSION}` exposed by [aui_module](@ref docs/aui_module.md) and [aui_executable](@ref docs/aui_executable.md).

`CMAKE_PROJECT_VERSION` is typically defined by [project](https://cmake.org/cmake/help/latest/command/project.html) 
CMake command:

@snippet test/minimal_deployment_test/CMakeLists.txt AUI_CMAKE_PROJECT_VERSION

## API_\<module name\>

\c dllexport (on Windows) or \c visibility (on other platforms) policy for the symbol.

@code{cpp}
class API_AUI_VIEWS AView ... { // defined in aui.views module
  ...
};
@endcode