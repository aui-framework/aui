cmake_minimum_required(VERSION 3.15)

set(_SAVED_AUIB_SDL3_VALIDATE ${AUIB_SDL3_VALIDATE})
set(AUIB_SDL3_VALIDATE OFF)
auib_import(SDL3 https://github.com/libsdl-org/SDL VERSION release-3.2.26)
set(AUIB_SDL3_VALIDATE ${_SAVED_AUIB_SDL3_VALIDATE})

aui_executable(aui.example.embedded_sdl)
aui_link(aui.example.embedded_sdl PRIVATE aui::core aui::views)
