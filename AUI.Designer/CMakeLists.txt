cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID)
    if (TARGET AUI.Views)
        add_subdirectory(registrator)

        AUI_Executable(AUI.Designer)
        AUI_Compile_Assets(AUI.Designer)
        target_link_libraries(AUI.Designer PRIVATE AUI.Core)
        target_link_libraries(AUI.Designer PRIVATE AUI.Data)
        target_link_libraries(AUI.Designer PRIVATE AUI.Views)
        target_link_libraries(AUI.Designer PRIVATE AUI.Crypt)
        target_link_libraries(AUI.Designer PRIVATE AUI.Designer.Registrator)
    endif()
else()
    message("Not building AUI.Designer because of not native platform. Please specify AUI_SDK_BIN with containing folder
             of your AUI.Toolbox.")
endif()