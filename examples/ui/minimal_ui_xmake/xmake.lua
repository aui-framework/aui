-- Specify available build configurations
add_rules("mode.release", "mode.debug")
-- Specify compile commands output directory and LSP to analyze C++ code files and highlight IntelliSense
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode", lsp = "clangd"})

-- Specify C++ standard to use, as AUI uses C++20 by default
set_languages("c++20")

-- Download aui package to use for targets later
add_requires("aui")

-- Define our target executable to build
target("aui-minimal-example")
    -- Add source code and headers to target
    add_files("src/*.cpp")
    add_includedirs("src")
    -- Add AUI package to target while linking only required components
    add_packages("aui", {components = {"core", "image", "views", "xml"}})
    -- Resolve linking by grouping AUI components into link groups
    add_linkgroups("aui.views", "aui.xml", "aui.image", "aui.core", {whole = true})
