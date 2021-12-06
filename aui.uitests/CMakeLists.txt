cmake_minimum_required(VERSION 3.10)

find_package(Boost COMPONENTS unit_test_framework)
if(Boost_FOUND)
    aui_module(aui.uitests ${SQLITE3_SRCS})
    target_link_libraries(aui.uitests PRIVATE aui.core aui.views ${Boost_LIBRARIES})
    target_include_directories(aui.uitests PRIVATE ${Boost_INCLUDE_DIRS})
endif()