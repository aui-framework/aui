cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID)
    if (TARGET aui.views)
        aui_module(aui.designer.registrator)
        target_link_libraries(aui.designer.Registrator PRIVATE aui.core)
        target_link_libraries(aui.designer.Registrator PRIVATE aui.views)
        target_link_libraries(aui.designer.Registrator PRIVATE aui.crypt)
    endif()
else()
    message("Not building aui.designer.Registrator because of not native platform. Please specify AUI_SDK_BIN with containing folder
             of your aui.toolbox.")
endif()