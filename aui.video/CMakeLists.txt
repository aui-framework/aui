cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE EXTRA_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

aui_module(aui.video EXPORT aui ADDITIONAL_SRCS ${EXTRA_SRCS})
aui_link(aui.video PUBLIC aui::core aui::image)
add_subdirectory(3rdparty/webm_parser)
target_link_directories(aui.video PUBLIC /home/ilyazavalov/msonline/libvpx)
target_include_directories(aui.video PUBLIC /home/ilyazavalov/msonline/libvpx)
target_link_libraries(aui.video PRIVATE vpx)
