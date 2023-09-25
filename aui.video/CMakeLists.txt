cmake_minimum_required(VERSION 3.16)

aui_module(aui.video EXPORT aui)

aui_link(aui.video PUBLIC aui::core aui::audio aui::image)

#auib_import(libvpx https://github.com/IlyaZavalov/libvpx.git ADD_SUBDIRECTORY
#        VERSION 5c803b9e2cc32da0d861ee0c802cb9798d62e574)

#aui_link(aui.video PRIVATE vpx vpxtools)

add_subdirectory(3rdparty/libwebm)
add_subdirectory(3rdparty/libvpx)
add_subdirectory(3rdparty/libyuv)


