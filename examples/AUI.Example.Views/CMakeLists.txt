cmake_minimum_required(VERSION 3.15)

if (NOT TARGET aui::views)

    # Use AUI.Boot
    file(
            DOWNLOAD
            https://raw.githubusercontent.com/aui-framework/aui/develop/aui.boot.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

    # import AUI
    auib_import(AUI https://github.com/aui-framework/aui
            VERSION develop
            COMPONENTS core views)

endif()

aui_executable(aui.example.views)
aui_compile_assets(aui.example.views)


aui_link(aui.example.views PRIVATE aui::core aui::views)


aui_app(TARGET aui.example.views
        NAME "AUI Examples"
        APPLE_BUNDLE_IDENTIFIER "aui.Examples"
        APPLE_SIGN_IDENTITY "iPhone Developer"
        APPLE_TEAM_ID "98S256N2KJ")
