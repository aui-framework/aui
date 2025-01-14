cmake_minimum_required(VERSION 3.24)

project(test_project)

set(AUIB_FORCE_PRECOMPILED ON CACHE BOOL "Force used precompiled binaries")
set(AUI_TEST_DEPLOY_VERSION v6.2.1 CACHE BOOL "AUI version")

include(FetchContent)
FetchContent_Declare(aui
    URL https://github.com/aui-framework/aui/releases/download/${AUI_TEST_DEPLOY_VERSION}/aui_linux-gnu-x86_64-static-debug.tar.gz)

FetchContent_MakeAvailable(aui)
FetchContent_GetProperties(aui
        SOURCE_DIR aui_ROOT
)
find_package(aui REQUIRED)

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core aui::views)
aui_compile_assets(test_project)
