cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.hot_code_reload)
aui_link(aui.example.hot_code_reload PRIVATE aui::core aui::views aui::remote_tools)
target_compile_options(aui.example.hot_code_reload PRIVATE
        -falign-functions=4096
        -ffunction-sections
        -fdata-sections
        -fno-lto
)
#target_link_options(aui.example.hot_code_reload PRIVATE
#        -fuse-ld=gold
#        -Wl,--gdb-index
#        -Wl,--incremental
#        -Wl,-z,norelro
#        -fno-lto
#)

function(aui_enable_hot_reload AUI_MODULE_NAME)
    _aui_find_root()
    set(LDSCRIPT_OUT "${CMAKE_CURRENT_BINARY_DIR}/aui.hot_code_reload.linker.ld")

#    file(WRITE ${LDSCRIPT_OUT} [[
#SECTIONS
#{
#}
#INSERT BEFORE .text;
#    ]])

    target_link_options(aui.example.hot_code_reload PRIVATE "-T${LDSCRIPT_OUT}")

    set(SCRIPT ${AUI_BUILD_AUI_ROOT}/cmake/hot_code_reload/generate_ld.sh)

#    add_custom_command(
#            TARGET ${AUI_MODULE_NAME} POST_BUILD
#            COMMAND ${SCRIPT} $<TARGET_FILE:${AUI_MODULE_NAME}> ${LDSCRIPT_OUT}
#            COMMENT "Generating symbol addresses for linker script"
#            DEPENDS ${SCRIPT}
#    )

endfunction()

aui_enable_hot_reload(aui.example.hot_code_reload)

