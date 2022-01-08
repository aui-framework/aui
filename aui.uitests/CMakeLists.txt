cmake_minimum_required(VERSION 3.10)

find_package(Boost COMPONENTS unit_test_framework)
if(Boost_FOUND)
    aui_module(aui.uitests EXPORT aui)
    target_include_directories(aui.uitests PRIVATE ${Boost_INCLUDE_DIRS})
    aui_link(aui.uitests PRIVATE aui.core aui.views Boost::unit_test_framework)
endif()