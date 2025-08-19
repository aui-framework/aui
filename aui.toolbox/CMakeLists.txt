cmake_minimum_required(VERSION 3.10)

if(ANDROID OR IOS)
    message("Not building aui.toolbox because of not native platform. Please specify AUI_DIR with bin folder with aui.toolbox")
    return()
endif()

aui_executable(aui.toolbox WIN32_SUBSYSTEM_CONSOLE EXPORT aui)
if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    aui_link(aui.toolbox PRIVATE aui::core aui::crypt aui::image )
else()
    aui_link(aui.toolbox PRIVATE aui::core aui::crypt aui::image)
endif()
aui_enable_tests(aui.toolbox)