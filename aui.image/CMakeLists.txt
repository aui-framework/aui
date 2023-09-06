cmake_minimum_required(VERSION 3.10)

auib_import(lunasvg https://github.com/aui-framework/lunasvg
            VERSION 18d3898983a337e63b459e63c402a807d914cecb)

aui_module(aui.image WHOLEARCHIVE EXPORT aui)
target_include_directories(aui.image PRIVATE 3rdparty)
aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg)
aui_enable_tests(aui.image)
