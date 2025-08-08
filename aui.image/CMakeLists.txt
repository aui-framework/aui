cmake_minimum_required(VERSION 3.10)

auib_import(lunasvg https://github.com/sammycage/lunasvg
            VERSION v3.3.0
            CMAKE_ARGS -DLUNASVG_BUILD_EXAMPLES=OFF -DPLUTOVG_BUILD_EXAMPLES=OFF
)


set(WEBP_COMPONENTS_TO_DISABLE ANIM_UTILS CWEBP DWEBP GIF2WEBP IMG2WEBP VWEBP WEBPINFO LIBWEBPMUX WEBPMUX EXTRAS)

foreach(_component ${WEBP_COMPONENTS_TO_DISABLE})
    list(APPEND WEBP_CMAKE_ARGS "-DWEBP_BUILD_${_component}=OFF")
endforeach()

auib_import(WebP https://github.com/webmproject/libwebp VERSION v1.3.1 CMAKE_ARGS ${WEBP_CMAKE_ARGS})

aui_module(aui.image WHOLEARCHIVE EXPORT aui)

# Wait for https://github.com/nothings/stb/pull/1619 being merged
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/stb_image_write.h" FILE_CONTENT)
set(OLD_SPRINTF_STRING
"#ifdef __STDC_LIB_EXT1__\n\
        len = sprintf_s(buffer, sizeof(buffer), \"EXPOSURE=          1.0000000000000\\n\\n-Y %d +X %d\\n\", y, x);\n\
#else\n\
        len = sprintf(buffer, \"EXPOSURE=          1.0000000000000\\n\\n-Y %d +X %d\\n\", y, x);\n\
#endif")
set(NEW_SPRINTF_STRING
"#ifdef __STDC_LIB_EXT1__\n\
        len = sprintf_s(buffer, sizeof(buffer), \"EXPOSURE=          1.0000000000000\\n\\n-Y %d +X %d\\n\", y, x);\n\
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L\n\
        len = snprintf(buffer, sizeof(buffer), \"EXPOSURE=          1.0000000000000\\n\\n-Y %d +X %d\\n\", y, x);\n\
#else\n\
        len = sprintf(buffer, \"EXPOSURE=          1.0000000000000\\n\\n-Y %d +X %d\\n\", y, x);\n\
#endif")
string(REPLACE "${OLD_SPRINTF_STRING}" "${NEW_SPRINTF_STRING}" FILE_CONTENT "${FILE_CONTENT}")
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/stb_image_write.h" "${FILE_CONTENT}")

add_subdirectory(3rdparty)
aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg WebP::webp WebP::webpdemux)
aui_enable_tests(aui.image)
