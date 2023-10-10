cmake_minimum_required(VERSION 3.16)

aui_module(aui.video EXPORT aui)
aui_link(aui.video PUBLIC aui::core aui::audio aui::image)

auib_import(libvpx https://github.com/IlyaZavalov/libvpx.git ADD_SUBDIRECTORY VERSION d84644a)
add_dependencies(aui.video libvpx)

aui_link(aui.video PRIVATE yuv vpx)
add_subdirectory(3rdparty/libwebm)