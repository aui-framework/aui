cmake_minimum_required(VERSION 3.10)

auib_import(lunasvg https://github.com/sammycage/lunasvg
            VERSION v2.3.5
            ADD_SUBDIRECTORY)

aui_module(aui.image WHOLEARCHIVE EXPORT aui)
target_include_directories(aui.image PRIVATE 3rdparty)
aui_link(aui.image PRIVATE aui::core lunasvg)
aui_enable_tests(aui.image)
