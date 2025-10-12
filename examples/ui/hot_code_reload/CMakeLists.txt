cmake_minimum_required(VERSION 3.10)

if (NOT AUI_PLATFORM_LINUX)
    return()
endif ()

# [aui_enable_hotswap]

aui_executable(aui.example.hot_code_reload)
aui_link(aui.example.hot_code_reload PRIVATE aui::core aui::views aui::remote_tools)
target_compile_options(aui.example.hot_code_reload PRIVATE
        -falign-functions=4096
#        -ffunction-sections
#        -fdata-sections
#        -fno-lto
)
#target_link_options(aui.example.hot_code_reload PRIVATE
#        -fuse-ld=gold
#        -Wl,--gdb-index
#        -Wl,--incremental
#        -Wl,-z,norelro
#        -fno-lto
#)

aui_enable_hotswap(aui.example.hot_code_reload)
# [aui_enable_hotswap]
