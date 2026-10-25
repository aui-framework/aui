cmake_minimum_required(VERSION 3.16)
project(aui-remote
        VERSION 0.0.1 # CI_PROJECT_VERSION
    )

option(BUILD_SHARED_LIBS "Build shared libs" OFF)
option(AUIB_FORCE_PRECOMPILED "AUI.Boot: force precompiled dependencies" OFF)
option(AUIB_NO_PRECOMPILED "AUI.Boot: disable precompiled dependencies" OFF)

# AUI to build against. By default, a released version is downloaded by AUI.Boot. To use a local checkout of AUI
# (i.e., to develop the framework and this app together), configure with -DAUI_LOCAL_PATH=/path/to/aui.
set(AUI_VERSION v8.0.0-rc.21)
set(AUI_LOCAL_PATH "" CACHE PATH "Path to a local AUI checkout; empty to use AUI_VERSION")

include(aui.boot.cmake)

if (AUI_LOCAL_PATH)
    auib_import(aui ${AUI_LOCAL_PATH} ADD_SUBDIRECTORY
            COMPONENTS core views curl json image crypt network
            VERSION local) # any version that is not cached by AUI.Boot as a download
else ()
    auib_import(aui https://github.com/aui-framework/aui
            COMPONENTS core views curl json image crypt network
            VERSION ${AUI_VERSION})
endif ()

aui_executable(${PROJECT_NAME})

aui_link(${PROJECT_NAME} PRIVATE aui::core aui::views aui::curl aui::json aui::image aui::crypt aui::network)

aui_compile_assets(${PROJECT_NAME})

aui_enable_tests(${PROJECT_NAME})

aui_app(TARGET ${PROJECT_NAME}
        NAME "AUI Remote"
        VENDOR "AUI Project"
)
