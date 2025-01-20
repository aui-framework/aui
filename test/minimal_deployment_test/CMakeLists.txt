cmake_minimum_required(VERSION 3.24)

project(test_project)

set(AUIB_FORCE_PRECOMPILED ON CACHE BOOL "Force used precompiled binaries")
set(AUI_TEST_PACKAGE "" CACHE BOOL "AUI tar.gz")
set(ARTIFACTS_DIR "" CACHE BOOL "artifacts directory")

if (NOT AUI_TEST_PACKAGE)
    message(FATAL_ERROR "AUI_TEST_PACKAGE is required")
endif()

include(FetchContent)

file(GLOB AUI_TEST_PACKAGE "${ARTIFACTS_DIR}/*.gz")

FetchContent_Declare(aui
        URL file://${AUI_TEST_PACKAGE})

FetchContent_MakeAvailable(aui)
FetchContent_GetProperties(aui
        SOURCE_DIR aui_ROOT
)
find_package(aui REQUIRED)


aui_executable(test_project)
aui_link(test_project PUBLIC aui::core aui::views)
aui_compile_assets(test_project)

file(GLOB children RELATIVE "${aui_ROOT}/bin" "${aui_ROOT}/bin/*")
if ((NOT "aui.toolbox" IN_LIST children) AND (NOT "aui.toolbox.exe" IN_LIST children))
    message(FATAL_ERROR "aui.toolbox is not present: ${children}")
endif()

foreach (file ${children})
    if (NOT file MATCHES "(aui.toolbox)")
        message(FATAL_ERROR "unexpected file ${file} in bin/ directory")
    endif ()
endforeach ()


aui_app(TARGET test_project
        NAME "Test Project")