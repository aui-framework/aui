cmake_minimum_required(VERSION 3.10)

aui_module(aui.updater EXPORT aui)
aui_link(aui.updater PRIVATE aui::core aui::crypt aui::curl aui::json)

if (TARGET aui.uitests)
    aui_enable_tests(aui.updater)
endif()

aui_get_package_name_and_arch(_default_qualifier)
target_compile_definitions(aui.updater PRIVATE AUI_DEFAULT_PACKAGE_QUALIFIER="${_default_qualifier}-portable")