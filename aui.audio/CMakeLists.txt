cmake_minimum_required(VERSION 3.10)

aui_module(aui.audio WHOLEARCHIVE EXPORT aui)
add_subdirectory(3rdparty/ogg)
aui_link(aui.audio PUBLIC aui::core aui::views)
aui_link(aui.audio PUBLIC ogg)

if (ANDROID)
    auib_import(oboe https://github.com/google/oboe ADD_SUBDIRECTORY VERSION main)
    aui_link(aui.audio PUBLIC oboe)
endif()