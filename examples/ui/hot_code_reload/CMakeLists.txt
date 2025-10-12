cmake_minimum_required(VERSION 3.10)

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

function(aui_enable_hotswap AUI_MODULE_NAME)
    if (NOT AUI_PLATFORM_LINUX)
        message(FATAL_ERROR "Hot code reload (hotswap) is supported on the following platforms only: Linux")
    endif ()
    string(SHA1 hash ${AUI_MODULE_NAME})
    set(hash "HotswapEnabler${hash}")
    file(GENERATE
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/hotswap_enable.cpp"
            CONTENT "#include <AUI/Remote/AHotCodeReload.h>\nstruct ${hash} { ${hash}() { AHotCodeReload::inst().addFiles(\"$<TARGET_OBJECTS:${AUI_MODULE_NAME}>\"); } } hotswapenabler${hash};"
    )
    target_sources(${AUI_MODULE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/hotswap_enable.cpp)
endfunction()

aui_enable_hotswap(aui.example.hot_code_reload)
