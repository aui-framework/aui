cmake_minimum_required(VERSION 3.10)


file(GLOB_RECURSE SQLITE3_SRCS 3rdparty/*.c)

aui_module(aui.sqlite ADDITIONAL_SRCS ${SQLITE3_SRCS}
                      EXPORT aui
                      PLUGIN)
target_include_directories(aui.sqlite PRIVATE 3rdparty/sqlite3)

aui_link(aui.sqlite PUBLIC aui::core)
aui_link(aui.sqlite PUBLIC aui::data)
if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    aui_link(aui.sqlite PRIVATE YY_Thunks)
endif()
aui_enable_tests(aui.sqlite)