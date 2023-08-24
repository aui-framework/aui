# AUI (Advanced Universal Interface)
![build badge](https://github.com/Alex2772/aui/actions/workflows/build.yml/badge.svg)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/9b8d9c80909a49ad8f171bb13a3bc675)](https://www.codacy.com/gh/Alex2772/aui/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Alex2772/aui&amp;utm_campaign=Badge_Grade)

![logo](https://raw.githubusercontent.com/aui-framework/aui/master/examples/AUI.Example.Views/assets/img/logo.svg)

Cross-platform high performance efficient module-based framework for developing and deploying hardware accelerated graphical desktop applications
using modern C++20.

[Documentation](https://aui-framework.github.io) | [Getting started](https://aui-framework.github.io/master/md_docs_Getting_started_with_AUI.html)

# Quickstart

## Linux dependencies

If you are using Linux, install following dependencies:

### Ubuntu
```bash
sudo apt install pkg-config libfontconfig-dev libxcursor-dev libxi-dev libxrandr-dev glew-devel libstdc++-static
```

### Fedora
```bash
sudo dnf install fontconfig-devel libXi libglvnd-devel libstdc++-static glew-devel
```

## Your project CMake script

To link AUI to your project, use the following CMake script:

```cmake
# Standard routine
cmake_minimum_required(VERSION 3.16)
project(aui_app)


# Tip: in a production project don't use branch name, use a specific name tag (i.e. v1.1.1), but for a sandbox project
# branch name is perfectly enough
set(AUI_VERSION master)

# Use AUI.Boot
file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

# import AUI
auib_import(aui https://github.com/aui-framework/aui
        COMPONENTS core views
        VERSION ${AUI_VERSION})


# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
aui_executable(${PROJECT_NAME})

# Link required libs
aui_link(${PROJECT_NAME} PRIVATE aui::core aui::views)

```

# Key features
- Extended common types (containers, strings, streams, etc...)
- Graphical User Interfaces, including framework tools for fast declarative UI producing using modern C++ capabilities
- Resource compiler (assets)
- Internationalization (i18n)
- DPI support
- CMake deployment scripts
- Model binding and ORM
- Encryption
- IO streams
- Networking, including http(s) requests
- DBMS integration including SQL query builder
- Both asynchronous and synchronous application architecture support

## Module list
- `aui.core` - basic types (containers, strings, streams, etc...)
- `aui.crypt` - wrapper around OpenSSL for encryption functionality
- `aui.curl` - wrapper around libcurl for http/https requests
- `aui.designer` - UI Designer (in dev)
- `aui.data` - DBMS interface and ORM
- `aui.image` - popular image format loader 
- `aui.json` - JSON parser
- `aui.mysql` - MySQL driver for `aui.data`
- `aui.network` - networking
- `aui.sqlite` - SQLite driver for `aui.data`
- `aui.svg` - SVG image loader
- `aui.toolbox` - resource compiler
- `aui.views` - UI toolkit
- `aui.xml` - XML parser

## Feature support
 - `-` equals unsupported
 - `?` equals planned
 - `+` equals almost completely supported
 - `#` equals fully supported

| Feature or module | Windows Vista/7/8/8.1/10 | Windows XP | Linux | Android | MacOS | iOS |
|-------------------|--------------------------|------------|-------|---------|-------|-----|
| aui.core          |             #            |     #      |   #   |    #    |   #   |  ?  |
| aui.crypt         |             #            |     #      |   #   |    #    |   #   |  ?  |
| aui.curl          |             #            |     #      |   #   |    #    |   #   |  ?  |
| aui.data          |             #            |     #      |   #   |    #    |   #   |  ?  |
| aui.designer      |             ?            |     ?      |   ?   |    -    |   ?   |  -  |
| aui.image         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.json          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.mysql         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.network       |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.sqlite        |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.svg           |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.toolbox       |             #            |     #      |   #   |    -    |   ?   |  -  |
| aui.views         |             #            |     ?      |   +   |    +    |   +   |  ?  |
| aui.xml           |             #            |     #      |   #   |    #    |   #   |  ?  |
| Assets            |             #            |     #      |   #   |    #    |   ?   |  ?  |
| Filesystem        |             #            |     #      |   #   |    #    |   ?   |  -  |
| Process creation  |             #            |     #      |   #   |    -    |   ?   |  -  |

| Compiler                     | Support |
|------------------------------|---------|
| MSVC 19+ (Visual Studio 20+) | Full    |
| gcc (8+)                     | Full    |
| MinGW (8+)                   | Partial |
| Cross-compile MinGW          | Partial |
| clang                        | Full    |

## Used libraries
- `libcurl` for http/https requests
- `OpenSSL` for encryption
- `OpenGL` as graphics hardware acceleration backend
- `glm` for linear algebra
- `stbimage` for image loading
- `freetype2` for font rendering


## Examples
Check the `examples/` folder.

![Fractal](https://sun9-42.userapi.com/impf/WruyOdMmMBrRfpjJ7QrhFepZj7obL3VMGxNSaw/Tr8XxKqdVV8.jpg?size=1261x740&quality=96&proxy=1&sign=f6b851a26a7c40a5f1c22367a34f4c71&type=album)

![Views](https://sun9-37.userapi.com/impg/1JYHdZ7PlYsCPvZnP3qeObUT4anFIH5GDghEEA/_JOtAwNfaLI.jpg?size=1261x1007&quality=96&sign=46300730d3b638ea9300e0238f8a511a&type=album)

![Minesweeper](https://sun9-10.userapi.com/impf/AW9aUF7nuKdkiOfEz7WtsKqhYARlwVaFb_qV0g/0EGtNBty3NI.jpg?size=392x481&quality=96&proxy=1&sign=adbaf47dada836ab25868abf8db9b9d5&type=album)

# Contributing
You are always free to contribute to this project and add yourself to the authors list :)
## Workflow
`master` branch contains code of the release candidate. Code of this branch must compile without errors and should pass
all tests on all supported platforms.
Each feature of the framework is created in a separate branch in the `feat/` folder. Pulling changes from your branch to
`master` is possible only if the merge with your branch does not generate merge conflicts and does not break the build
and tests.
