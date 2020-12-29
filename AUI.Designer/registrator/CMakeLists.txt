cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID)
    if (TARGET AUI.Views)
        AUI_Module(AUI.Designer.Registrator)
        target_link_libraries(AUI.Designer.Registrator PRIVATE AUI.Core)
        target_link_libraries(AUI.Designer.Registrator PRIVATE AUI.Views)
        target_link_libraries(AUI.Designer.Registrator PRIVATE AUI.Crypt)
    endif()
else()
    message("Not building AUI.Designer.Registrator because of not native platform. Please specify AUI_SDK_BIN with containing folder
             of your AUI.Toolbox.")
endif()