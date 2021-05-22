cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "Kara-Online")

if(NOT ANDROID)
    if (TARGET AUI.Views)
        AUI_Executable(AUI.Installer)
        AUI_Compile_Assets(AUI.Installer)
        target_link_libraries(AUI.Installer PRIVATE AUI.Core)
        target_link_libraries(AUI.Installer PRIVATE AUI.Data)
        target_link_libraries(AUI.Installer PRIVATE AUI.Views)
        target_link_libraries(AUI.Installer PRIVATE AUI.Crypt)
        target_compile_definitions(AUI.Installer PRIVATE PROJECT_NAME=\"${PROJECT_NAME}\")
    endif()
else()
    message("Not building AUI.Installer because of not native platform. Please specify AUI_SDK_BIN with containing folder
             of your AUI.Toolbox.")
endif()