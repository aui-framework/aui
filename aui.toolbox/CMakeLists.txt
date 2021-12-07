cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID AND NOT IOS)
    aui_executable(aui.toolbox)
    target_link_libraries(aui.toolbox PRIVATE aui.core)
    target_link_libraries(aui.toolbox PRIVATE aui.crypt)
else()
    message("Not building aui.toolbox because of not native platform. Please specify AUI_DIR with bin folder with aui.toolbox")
endif()