cmake_minimum_required(VERSION 3.10)

aui_module(aui.dev_extras EXPORT aui WHOLEARCHIVE)
aui_enable_tests(aui.dev_extras)

aui_link(aui.dev_extras PUBLIC aui::core aui::json)

# remote agent
aui_link(aui.dev_extras PRIVATE aui::views aui::curl)
