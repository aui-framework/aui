# Minimal UI Template XMake

@auiexample{ui}
Minimal UI boilerplate template XMake.

@image html docs/imgs/minimal-template.png

# Source Files
## Project Structure

@mermaid{project_structure_minimal_example_xmake}

---
## xmake.lua
```lua
-- Specify available build configurations
add_rules("mode.release", "mode.debug")
-- Specify compile commands output directory and LSP to analyze C++ code files and highlight IntelliSense
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode", lsp = "clangd"})

-- Specify C++ standard to use, as AUI uses C++20 by default
set_languages("c++20")

-- Download aui package to use for targets later
add_requires("aui")
-- Use shared GLEW to resolve LNK2019 errors
add_requireconfs("**.glew", {override = true, configs = {shared = true}})

-- Define our target executable to build
target("aui-minimal-example")
    -- Add source code and headers to target
    add_files("src/*.cpp")
    add_includedirs("src")
    -- Add AUI package to target while linking only required components
    add_packages("aui", {components = {"core", "image", "views", "xml"}})
    -- Add defines that point to your OS
    if is_plat("windows") then
        add_defines(
            "AUI_PLATFORM_WIN=1", "AUI_PLATFORM_LINUX=0", "AUI_PLATFORM_APPLE=0"
        )
    elseif is_plat("linux") then
        add_defines(
            "AUI_PLATFORM_WIN=0", "AUI_PLATFORM_LINUX=1", "AUI_PLATFORM_APPLE=0"
        )
    elseif is_plat("macosx") then
        add_defines(
            "AUI_PLATFORM_WIN=0", "AUI_PLATFORM_LINUX=0", "AUI_PLATFORM_APPLE=1"
        )
    end
    -- Add defines that point to symbol exports
    add_defines(
        "API_AUI_VIEWS=AUI_IMPORT", "API_AUI_IMAGE=AUI_IMPORT"
    )
    -- Link required syslinks
    add_syslinks("gdi32", "ole32")
```

## src/MainWindow.h

```cpp
#pragma once

#include <AUI/Platform/AWindow.h>

class MainWindow: public AWindow {
public:
    MainWindow();
};
```
## src/MainWindow.cpp
```cpp
#include "MainWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/Platform/APlatform.h>

using namespace declarative;

MainWindow::MainWindow(): AWindow("Project template app", 300_dp, 200_dp) {
    setContents(
        Centered{
            Vertical{
                Centered { Label { "Hello world from AUI!" } },
                _new<AButton>("Visit GitHub repo").connect(&AView::clicked, this, [] {
                    APlatform::openUrl("https://github.com/aui-framework/aui");
                }),
                _new<AButton>("Visit docs").connect(&AView::clicked, this, [] {
                    APlatform::openUrl("https://aui-framework.github.io/");
                }),
                _new<AButton>("Submit an issue").connect(&AView::clicked, this, [] {
                    APlatform::openUrl("https://github.com/aui-framework/aui/issues/new");
                }),
            }
        }
    );
}
```

## src/main.cpp
```cpp
#include <AUI/Platform/Entry.h>
#include "MainWindow.h"

AUI_ENTRY {
    _new<MainWindow>()->show();
    return 0;
};
```

## .vscode/c_cpp_properties.json
```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "windowsSdkVersion": "10.0.26100.0",
            "compilerPath": "cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++20",
            "intelliSenseMode": "windows-msvc-x64"
        }
    ],
    "version": 4
}
```

This example is located outside AUI's source tree. Checkout its
[repository](https://github.com/aui-framework/example_minimal_ui_xmake).
