cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID AND NOT IOS)
    aui_executable(aui.toolbox)
    aui_link(aui.toolbox PRIVATE aui.core)
    aui_link(aui.toolbox PRIVATE aui.crypt)
else()
    message("Not building aui.toolbox because of not native platform. Please specify AUI_DIR with bin folder with aui.toolbox")
endif()