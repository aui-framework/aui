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
add_subdirectory(3rdparty)
aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg WebP::webp WebP::webpdemux)
aui_enable_tests(aui.image)
