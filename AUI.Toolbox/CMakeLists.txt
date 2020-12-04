cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID)
    AUI_Executable(AUI.Toolbox)
    target_link_libraries(AUI.Toolbox PRIVATE AUI.Core)
    target_link_libraries(AUI.Toolbox PRIVATE AUI.Crypt)
else()
    message("Not building AUI.Toolbox because of not native platform. Please specify AUI_SDK_BIN with containing folder
             of your AUI.Toolbox.")
endif()