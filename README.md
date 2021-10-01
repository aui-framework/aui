# AUI (Advanced Universal Interface)
![build badge](https://github.com/Alex2772/aui/actions/workflows/build.yml/badge.svg)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/9b8d9c80909a49ad8f171bb13a3bc675)](https://www.codacy.com/gh/Alex2772/aui/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Alex2772/aui&amp;utm_campaign=Badge_Grade)

Cross-platform high performance efficient module-based framework for developing and deploying hardware accelerated graphical desktop applications
using modern C++17.

[Download](https://github.com/Alex2772/aui/releases/) | [Documentation](docs/MANUAL.md)

## Key features
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
- `AUI.Core` - basic types (containers, strings, streams, etc...)
- `AUI.Crypt` - wrapper around OpenSSL for encryption functionality
- `aui.curl` - wrapper around libcurl for http/https requests
- `aui.designer` - UI Designer (in dev)
- `aui.data` - DBMS interface and ORM
- `aui.image` - popular image format loader 
- `aui.json` - JSON parser
- `AUI.Mysql` - MySQL driver for `aui.data`
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
| AUI.Core          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Crypt         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.curl          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.data          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.designer      |             ?            |     ?      |   ?   |    -    |   ?   |  -  |
| aui.image         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.json          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Mysql         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.network       |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.sqlite        |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.svg           |             #            |     #      |   #   |    #    |   ?   |  ?  |
| aui.toolbox       |             #            |     #      |   #   |    -    |   ?   |  -  |
| aui.views         |             #            |     ?      |   +   |    +    |   ?   |  ?  |
| aui.xml           |             #            |     #      |   #   |    #    |   ?   |  ?  |
| Assets            |             #            |     #      |   #   |    #    |   ?   |  ?  |
| Filesystem        |             #            |     #      |   #   |    #    |   ?   |  -  |
| Process creation  |             #            |     #      |   #   |    -    |   ?   |  -  |

| Compiler                     | Support       |
|------------------------------|---------------|
| MSVC 19+ (Visual Studio 17+) | Full          |
| gcc (8+)                     | Full          |
| MinGW (8+)                   | Full          |
| Cross-compile MinGW          | Full          |
| clang                        | Unknown       |

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

![Views](https://sun9-72.userapi.com/impf/i-t02B1DzF1AJDBLpDrjX6QkjhnI6VruPGYgrA/8eU7ZvOZ2Lo.jpg?size=1261x740&quality=96&proxy=1&sign=06f90177b68f83228c529d5108e1b685&type=album)

![Minesweeper](https://sun9-10.userapi.com/impf/AW9aUF7nuKdkiOfEz7WtsKqhYARlwVaFb_qV0g/0EGtNBty3NI.jpg?size=392x481&quality=96&proxy=1&sign=adbaf47dada836ab25868abf8db9b9d5&type=album)

# Getting started

Please check [docs/MANUAL.md](docs/MANUAL.md)


# Contributing
You are always free to contribute to this project and add yourself to the authors list :)
## Workflow
`master` branch contains code of the release candidate. Code of this branch must compile without errors and should pass
all tests on all supported platforms.
Each feature of the framework is created in a separate branch in the `feat/` folder. Pulling changes from your branch to
`master` is possible only if the merge with your branch does not generate merge conflicts and does not break the build
and tests.

## Code style
AUI's code should be kept with the following code style:
- `No tabs`; only four spaces
- Row length: up to `120` characters
- Class names: `CamelCase`. Every framework class name starts with capital '`A`'. Use `GenericSpecific` pattern
  (`EventClose`, not `CloseEvent`, `PacketConnect`, not `ConnectPacket`). Using that convention files group nicely in
  file lists.
- Functions, variables: `lowerCamelCase`
- Constants: `UPPER_SNAKE_CASE`
- Getters: `get...`/`is...` (`lineNumber` -> `getLineNumber()`, `accessible` -> `isAccessible()`)
- Getters: `set...`/`with...` (`lineNumber` -> `setLineNumber(...)`, `accessible` -> `withAccessible(...)`)
- Structures: commonly holds some data and does not have member functions; `CamelCase`.
              Also used for template-based functionality (like `stl`, `AUI/Traits/`), in that case, `snake_case` used
              for file names, struct name and its member functions
