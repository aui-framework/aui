cmake_minimum_required(VERSION 3.15)

set(AUIB_SDL3_VALIDATE OFF)
auib_import(SDL3 https://github.com/libsdl-org/SDL VERSION release-3.2.26)

aui_executable(aui.example.embedded_sdl)
aui_link(aui.example.embedded_sdl PRIVATE aui::core aui::views)

if (TARGET SDL3::SDL3-static)
    aui_link(aui.example.embedded_sdl PRIVATE SDL3::SDL3-static)
elseif (TARGET SDL3::SDL3-shared)
    aui_link(aui.example.embedded_sdl PRIVATE SDL3::SDL3-shared)
endif ()
