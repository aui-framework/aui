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
    -- Add defines that point to your OS and link against required syslinks/frameworks
    if is_plat("windows") then
        add_defines(
            "AUI_PLATFORM_WIN=1", "AUI_PLATFORM_LINUX=0", "AUI_PLATFORM_APPLE=0"
        )
        add_syslinks("gdi32", "ole32")
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
