cmake_minimum_required(VERSION 3.10)

project(test_project)

FetchContent_Declare(aui
        https://github.com/aui-framework/aui/releases/download/v7.0.0-rc.2/aui_linux-gnu-x86_64-static-debug.tar.gz)

FetchContent_MakeAvailable(aui)

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core aui::views)
aui_compile_assets(test_project)
