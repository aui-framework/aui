cmake_minimum_required(VERSION 3.10)

project(test_project)

include(../../../../aui.boot.cmake)
set(AUIB_DEBUG_PRECOMPILED aui)
auib_import(aui https://github.com/aui-framework/aui
            COMPONENTS core views
            VERSION ${AUI_TEST_DEPLOY_VERSION})

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core aui::views)
aui_compile_assets(test_project)
