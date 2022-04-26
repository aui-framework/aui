cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID AND NOT IOS)
    aui_executable(aui.toolbox WIN32_SUBSYSTEM_CONSOLE)
    aui_link(aui.toolbox PRIVATE aui::core aui::crypt aui::image)
else()
    message("Not building aui.toolbox because of not native platform. Please specify AUI_DIR with bin folder with aui.toolbox")
endif()