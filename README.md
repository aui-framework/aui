# AUI (Advanced Universal Interface)
Cross-platform high performance efficient module-based framework for developing and deploying hardware accelerated graphical desktop applications
using modern C++17.

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
- `AUI.Curl` - wrapper around libcurl for http/https requests
- `AUI.Designer` - UI Designer (in dev)
- `AUI.Data` - DBMS interface and ORM
- `AUI.Image` - popular image format loader 
- `AUI.Json` - JSON parser
- `AUI.Mysql` - MySQL driver for `AUI.Data`
- `AUI.Network` - networking
- `AUI.Sqlite` - SQLite driver for `AUI.Data`
- `AUI.Svg` - SVG image loader
- `AUI.Toolbox` - resource compiler
- `AUI.Views` - UI toolkit
- `AUI.Xml` - XML parser

## Feature support
 - `-` equals unsupported
 - `?` equals planned
 - `+` equals almost completely supported
 - `#` equals fully supported

| Feature or module | Windows Vista/7/8/8.1/10 | Windows XP | Linux | Android | MacOS | iOS |
|-------------------|--------------------------|------------|-------|---------|-------|-----|
| AUI.Core          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Crypt         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Curl          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Data          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Designer      |             ?            |     ?      |   ?   |    -    |   ?   |  -  |
| AUI.Image         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Json          |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Mysql         |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Network       |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Sqlite        |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Svg           |             #            |     #      |   #   |    #    |   ?   |  ?  |
| AUI.Toolbox       |             #            |     #      |   #   |    -    |   ?   |  -  |
| AUI.Views         |             #            |     ?      |   +   |    +    |   ?   |  ?  |
| AUI.Xml           |             #            |     #      |   #   |    #    |   ?   |  ?  |
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
