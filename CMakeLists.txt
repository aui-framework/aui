cmake_minimum_required(VERSION 3.16)
project(aui-remote
        VERSION 0.0.1 # CI_PROJECT_VERSION
    )

option(BUILD_SHARED_LIBS "Build shared libs" OFF)
option(AUIB_FORCE_PRECOMPILED "AUI.Boot: force precompiled dependencies" OFF)
option(AUIB_NO_PRECOMPILED "AUI.Boot: disable precompiled dependencies" OFF)

include(aui.boot.cmake)

auib_import(aui https://github.com/aui-framework/aui
        COMPONENTS core views curl json image crypt network dev_extras
        VERSION 3325c1525d53a5e400596857f8622b4173236b1b)

aui_executable(${PROJECT_NAME})

aui_link(${PROJECT_NAME} PRIVATE aui::core aui::views aui::curl aui::json aui::image aui::crypt aui::network)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # a debug build of the GUI can be driven by aui-remote itself, see README.md
    aui_link(${PROJECT_NAME} PRIVATE aui::dev_extras)
endif ()

aui_compile_assets(${PROJECT_NAME})

aui_enable_tests(${PROJECT_NAME})

aui_app(TARGET ${PROJECT_NAME}
        ID "com.github.aui-remote"
        NAME "AUI Remote"
        VENDOR "AUI Project"
)
