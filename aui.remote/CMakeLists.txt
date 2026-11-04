cmake_minimum_required(VERSION 3.16)

# aui-remote, the client of the remote agent of aui::dev_extras: a GUI and a CLI for humans and AI agents. A tool for
# the host, like aui.toolbox.
foreach (_dep core views curl json image crypt network)
    if (NOT TARGET aui.${_dep})
        message(STATUS "Not building aui.remote: aui.${_dep} is not built")
        return()
    endif ()
endforeach ()

aui_executable(aui.remote EXPORT aui)
set_target_properties(aui.remote PROPERTIES OUTPUT_NAME aui-remote)
aui_link(aui.remote PRIVATE aui::core aui::views aui::curl aui::json aui::image aui::crypt aui::network)
aui_compile_assets(aui.remote)
aui_enable_tests(aui.remote)

get_filename_component(_aui_root "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
if (NOT CMAKE_SOURCE_DIR STREQUAL _aui_root)
    # AUI is a subdirectory of a consumer's project (AUIB_AUI_AS): build aui-remote only when asked for
    # (cmake --build . -t aui.remote); otherwise the consumer takes it from the distribution of AUI
    set_target_properties(aui.remote PROPERTIES EXCLUDE_FROM_ALL ON)
endif ()

# aui-remote drives itself: a debug build of its GUI starts the remote agent. Only in a build of AUI itself (not one made
# by AUI.Boot for a consumer), so that an aui-remote of a consumer doesn't show up among the applications of another one.
if (CMAKE_SOURCE_DIR STREQUAL _aui_root AND NOT AUI_BOOT_INSIDE AND CMAKE_BUILD_TYPE STREQUAL "Debug"
        AND TARGET aui.dev_extras)
    set(_self_debug ON)
else ()
    set(_self_debug OFF)
endif ()
option(AUI_REMOTE_SELF_DEBUG "aui-remote GUI starts the remote agent, so aui-remote can drive itself" ${_self_debug})
if (AUI_REMOTE_SELF_DEBUG)
    aui_link(aui.remote PRIVATE aui::dev_extras)
    target_compile_definitions(aui.remote PRIVATE AUI_REMOTE_SELF_DEBUG=1)
endif ()
