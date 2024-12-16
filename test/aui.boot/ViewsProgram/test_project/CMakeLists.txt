cmake_minimum_required(VERSION 3.10)

project(test_project)

include(../../../../aui.boot.cmake)
auib_import(aui ../../../../
            COMPONENTS core views)

aui_executable(test_project)

get_target_property(_t GLEW::GLEW INTERFACE_LINK_LIBRARIES)
message(STATUS ${_t})

aui_link(test_project PUBLIC aui::core aui::views)
