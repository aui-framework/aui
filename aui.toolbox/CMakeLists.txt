cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID)
    aui_executable(aui.toolbox)
    target_link_libraries(aui.toolbox PRIVATE aui.core)
    target_link_libraries(aui.toolbox PRIVATE aui.crypt)
else()
    message("Not building aui.toolbox because of not native platform. Please specify AUI_SDK_BIN with containing folder
             of your aui.toolbox.")
endif()