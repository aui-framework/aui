cmake_minimum_required(VERSION 3.10)

auib_import(lunasvg https://github.com/aui-framework/lunasvg
            VERSION 272ceee)


set(WEBP_COMPONENTS_TO_DISABLE ANIM_UTILS CWEBP DWEBP GIF2WEBP IMG2WEBP VWEBP WEBPINFO LIBWEBPMUX WEBPMUX EXTRAS)

foreach(_component ${WEBP_COMPONENTS_TO_DISABLE})
    list(APPEND WEBP_CMAKE_ARGS "-DWEBP_BUILD_${_component}=OFF")
endforeach()

auib_import(WebP https://github.com/webmproject/libwebp VERSION 1.3.1 CMAKE_ARGS ${WEBP_CMAKE_ARGS})

aui_module(aui.image WHOLEARCHIVE EXPORT aui)
target_include_directories(aui.image PRIVATE 3rdparty)
aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg WebP::webp WebP::webpdemux)
aui_enable_tests(aui.image)
