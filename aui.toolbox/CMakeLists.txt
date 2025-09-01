cmake_minimum_required(VERSION 3.10)

if(ANDROID OR IOS)
    message("Not building aui.toolbox because of not native platform. Please specify AUI_DIR with bin folder with aui.toolbox")
    return()
endif()

aui_executable(aui.toolbox WIN32_SUBSYSTEM_CONSOLE EXPORT aui)
aui_link(aui.toolbox PRIVATE aui::core aui::crypt aui::image)
aui_enable_tests(aui.toolbox)
