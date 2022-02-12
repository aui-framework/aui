cmake_minimum_required(VERSION 3.10)

project(test_project)

include(../../../../aui.boot.cmake)
auib_import(AUI ../../../../
            COMPONENTS core)

find_package(ZLIB REQUIRED)

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core ZLIB::ZLIB)
