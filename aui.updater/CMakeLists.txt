cmake_minimum_required(VERSION 3.10)

aui_module(aui.updater EXPORT aui)
if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    aui_link(aui.updater PRIVATE aui::core aui::crypt aui::curl aui::json YY_Thunks)
else()
    aui_link(aui.updater PRIVATE aui::core aui::crypt aui::curl aui::json)
endif()

aui_get_package_name_and_arch(_default_qualifier)
target_compile_definitions(aui.updater PRIVATE AUI_DEFAULT_PACKAGE_QUALIFIER="${_default_qualifier}-portable")

if (TARGET aui.uitests)
    aui_enable_tests(aui.updater)
    if (NOT BUILD_SHARED_LIBS)
        target_compile_definitions(Tests PRIVATE AUI_UPDATER_TEST=1)
        add_subdirectory(aui.updater.test)
    endif()
endif()

