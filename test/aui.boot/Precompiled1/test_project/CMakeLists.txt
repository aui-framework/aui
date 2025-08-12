cmake_minimum_required(VERSION 3.10)

project(test_project)

include(../../../../aui.boot.cmake)
set(AUIB_DEBUG_PRECOMPILED aui)
# raise aui.boot validation level.
set(AUIB_VALIDATION_LEVEL 2 CACHE STRING "Package validation level")
if (DEFINED AUI_TEST_DEPLOY_REPO)
auib_import(aui ${AUI_TEST_DEPLOY_REPO}
            COMPONENTS core
            VERSION ${AUI_TEST_DEPLOY_VERSION})
else()
auib_import(aui https://github.com/aui-framework/aui
            COMPONENTS core
            VERSION ${AUI_TEST_DEPLOY_VERSION})
endif()

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core)
