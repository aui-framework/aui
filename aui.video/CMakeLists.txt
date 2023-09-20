cmake_minimum_required(VERSION 3.16)

aui_module(aui.video EXPORT aui)

aui_link(aui.video PUBLIC aui::core aui::audio aui::image)

add_subdirectory(3rdparty/libwebm)
add_subdirectory(3rdparty/libvpx)
add_subdirectory(3rdparty/libyuv)


