# AUI (Advanced Universal Interface)
![build badge](https://github.com/aui-framework/aui/actions/workflows/build.yml/badge.svg)
[![discord badge](https://dcbadge.limes.pink/api/server/https://discord.gg/jq2WySpg6m?style=flat)](https://discord.gg/jq2WySpg6m)

![logo](https://raw.githubusercontent.com/aui-framework/aui/master/logo.svg)

Cross-platform high performance efficient module-based framework for developing and deploying hardware accelerated graphical desktop applications
using modern C++20.

The project is inspired by Qt and aims to provide developers with the best possible experience (including, but not limited
to: dependency management, packaging, ui building, styling, debugger visualizing) with pure C++, without custom
programming languages and external compilers.

[Documentation](https://aui-framework.github.io) | [Examples](https://aui-framework.github.io/master/examples.html) | [Getting started](https://aui-framework.github.io/master/md_docs_2Getting_01started_01with_01AUI.html) | [Roadmap](https://github.com/orgs/aui-framework/projects/3/views/1) | [News](https://github.com/aui-framework/aui/discussions/categories/announcements) | [Discussions](https://github.com/aui-framework/aui/discussions)

# Quickstart

## Linux dependencies

If you are using Linux, install following dependencies:

### Ubuntu
```bash
sudo apt install pkg-config libfontconfig-dev libxcursor-dev libxi-dev libxrandr-dev libglew-dev libstdc++-static libpulse-dev libdbus-1-dev
```

### Fedora
```bash
sudo dnf install fontconfig-devel libXi libglvnd-devel libstdc++-static glew-devel pulseaudio-libs-devel
```

## AUI App Template ⚡

Use our setup-free repository templates for quick start:

1. [Minimal UI](https://github.com/aui-framework/example_minimal_ui) - an absolute minimum to start a graphical UI application, without any boilerplate.
2. [Minimal UI with assets](https://github.com/aui-framework/example_assets_ui) - same as above but with assets.
3. [Full-fledged App Template](https://aui-framework.github.io/master/example_app_template.html) - a complete template to create a GitHub-hosted app project with CI/CD building,
   testing, releasing, auto updating, code quality checking and more.

## Integrating AUI to existing CMake project

To link AUI to your project, use the following CMake script. This script is self sufficient and does not require additional setup, AUI is imported to your project thanks to
[AUI.Boot](https://aui-framework.github.io/master/md_docs_2AUI_01Boot.html).

`CMakeLists.txt`:
```cmake
# Standard routine
cmake_minimum_required(VERSION 3.16)
project(aui_app)

set(AUI_VERSION v7.0.1)

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


# Create the executable. This function automatically links all sources from the src/ folder,
# creates CMake target and places the resulting executable to bin/ folder.
aui_executable(${PROJECT_NAME})

# Link required libs
aui_link(${PROJECT_NAME} PRIVATE aui::core aui::views)
```

`src/main.cpp`:
```cpp
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/Platform/APlatform.h>

using namespace declarative;

AUI_ENTRY {
    auto w = _new<AWindow>("Window title", 300_dp, 200_dp);
    w->setContents(Centered {
      Vertical {
        Centered { Label { "Hello world from AUI!" } },
        Centered {
          Button { "Visit GitHub page" }.clicked(w, [] {
              APlatform::openUrl("https://github.com/aui-framework/aui");
          }),
        },
      },
    });
    w->show();
    return 0;
}
```

Result:

![Example window](https://github.com/aui-framework/aui/blob/develop/docs/imgs/Screenshot_20241218_144940.png?raw=true)


Visit [layout building page](https://aui-framework.github.io/master/group__layout__managers.html) and our [introduction
guide](https://aui-framework.github.io/master/md_docs_2Getting_01started_01with_01AUI.html) for more info.

Optionally, you can use one of [our IDE plugins](https://aui-framework.github.io/master/md_docs_2IDE_01Plugins.html) to set up the project.

# Key features
- Extended common types (containers, strings, streams, etc...)
- Graphical User Interfaces, including framework tools for fast declarative UI producing using modern C++ capabilities
- Resource compiler (assets)
- Internationalization (i18n)
- DPI support
- CMake deployment scripts
- Model binding
- Encryption
- IO streams
- Networking, including http(s) requests
- Both asynchronous and synchronous application architecture support

## Module list
- `aui.core` - basic types (containers, strings, streams, etc...)
- `aui.crypt` - wrapper around OpenSSL for encryption functionality
- `aui.curl` - wrapper around libcurl for http/https requests
- `aui.image` - popular image format loader 
- `aui.json` - JSON parser
- `aui.network` - networking
- `aui.toolbox` - resource compiler
- `aui.uitests` - UI testing based on GTest
- `aui.views` - UI toolkit
- `aui.xml` - XML parser

## Feature support
 - `-` equals unsupported
 - `?` equals planned
 - `+` equals almost completely supported
 - `#` equals fully supported

| Feature or module | Windows Vista+ | Windows XP | Linux | Android | MacOS | iOS |
|-------------------|----------------|------------|-------|---------|-------|-----|
| aui.core          | #              |     #      |   #   |    #    |   #   |  #  |
| aui.crypt         | #              |     #      |   #   |    #    |   #   |  #  |
| aui.curl          | #              |     #      |   #   |    #    |   #   |  #  |
| aui.image         | #              |     #      |   #   |    #    |   #   |  #  |
| aui.json          | #              |     #      |   #   |    #    |   #   |  #  |
| aui.network       | #              |     #      |   #   |    #    |   +   |  +  |
| aui.toolbox       | #              |     #      |   #   |    -    |   #   |  -  |
| aui.views         | #              |     ?      |   +   |    +    |   +   |  +  |
| aui.xml           | #              |     #      |   #   |    #    |   #   |  #  |
| Assets            | #              |     #      |   #   |    #    |   +   |  +  |
| Filesystem        | #              |     #      |   #   |    #    |   +   |  +  |
| Process creation  | #              |     #      |   #   |    -    |   ?   |  -  |

| Compiler                     | Support       |
|------------------------------|---------------|
| MSVC 19+ (Visual Studio 20+) | Full          |
| gcc (8+)                     | Full          |
| MinGW (8+)                   | Won't compile |
| Cross-compile MinGW          | Won't compile |
| clang                        | Full          |

## Used libraries
- [libcurl](https://curl.se/) for http/https requests
- [OpenSSL](https://github.com/openssl/openssl) for encryption
- `OpenGL` as graphics hardware acceleration backend
- [glm](https://github.com/g-truc/glm) for linear algebra
- [stbimage](https://github.com/nothings/stb) for image loading
- [freetype2](https://github.com/freetype/freetype) for font rendering
- [LunaSVG](https://github.com/sammycage/lunasvg) for SVG rendering
- [GTest](https://github.com/google/googletest) for unit testing
- [Google Benchmark](https://github.com/google/benchmark) for benchmark testing

## IDE Plugins

[Refer to documentation page](https://aui-framework.github.io/master/md_docs_2IDE_01Plugins.html)

## Projects using AUI

- [AUI Telegram Client](https://github.com/aui-framework/telegram_client)
  ![AUI Telegram Client](https://github.com/aui-framework/telegram_client/blob/f985af77711be2b17b6aab11bfafb2a864800a1e/demo/demo.jpg?raw=true)
- [Magicsea Online](https://magicseaonline.com/)
  ![Magicsea Online](https://github.com/aui-framework/aui/blob/develop/docs/imgs/owrfuihw34iosdfjnfj.jpg?raw=true)
- [Amplitude Studio](https://studio.amplitudeaudiosdk.com)
  ![Amplitude Studio](https://github.com/aui-framework/aui/blob/develop/docs/imgs/2njkb4fhjkcbjkw.jpg?raw=true)

## Examples

Check the [examples](https://aui-framework.github.io/master/examples.html) page in our
[docs](https://aui-framework.github.io) or [examples/](https://github.com/aui-framework/aui/tree/master/examples) dir in
our repo.

![Fractal](https://github.com/user-attachments/assets/6c67cfb2-adba-42e9-9201-0b87b2dbb3b8)

![Views](https://github.com/user-attachments/assets/f0071fe4-c7d4-4d02-b677-2c02be51edd1)

![Minesweeper](https://github.com/user-attachments/assets/3299da4c-6ae3-4be8-9714-f01622a38fb2)


# Licensing

Please refer to [LICENSE](https://github.com/aui-framework/aui/blob/master/LICENSE).

Our project is published under Mozilla Public License Version 2.0 (the License). In short, you can use AUI for free in
your private (i.e, that does not go outside your company) projects.

If you would like to publish your project (including, but not limited to: sell, sign, release in software stores like
Google Play or App Store), does not matter open source, closed source or proprietary, the License obligates you to:
 - Mention your project uses "AUI Framework" in your legal notice.
 - Release your changes to AUI (if any) under the same terms (the License).

As for the latter, you are welcome to upstream your changes (i.e., propose your changes to
[this repository](https://github.com/aui-framework/aui)).

You are not obligated to disclose your own sources nor release your own project under the same terms (License).

# Maintaining

The AUI framework is maintained by Alexey Titov ([Alex2772](https://github.com/Alex2772), alex2772sc 'at' gmail.com)
with contributions from many other people. Join our [Discord community](https://discord.gg/jq2WySpg6m) for voice
calls.

Let us know if your contribution is not listed or mentioned incorrectly, and we'll make it right.

The project evolves fast and inconsistently, possibly breaking backward compatibility. The APIs likely to be
changed/removed are marked in docs as experimental. Consult with our migration guides on release pages.

# Contributing

You are welcome to open issues and pull requests on our GitHub repository. This project is maintained by a team of
volunteers on a reasonable-effort basis. As such, we can accept your issue, but we can't guarantee resolution. It's all
depends on severity and our own needs. In fact, we'd be happy if you submit a pull request. In this case we'll do our
best to help you merge your changes to upstream.

Please refer to [CONTRIBUTING.md](https://github.com/aui-framework/aui/blob/master/CONTRIBUTING.md)

# Star History

[![Star History Chart](https://api.star-history.com/svg?repos=aui-framework/aui&type=Date)](https://star-history.com/#aui-framework/aui&Date)
