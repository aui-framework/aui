cmake_minimum_required(VERSION 3.10)

auib_import(lunasvg https://github.com/aui-framework/lunasvg
            VERSION 18d3898983a337e63b459e63c402a807d914cecb)

auib_import(WebP https://github.com/IlyaZavalov/libwebp
        VERSION 9926bcfc8c7148d0229d3771858dc0566a00aa42
        )

find_package(WebP)

aui_module(aui.image WHOLEARCHIVE EXPORT aui)
target_include_directories(aui.image PRIVATE 3rdparty)
aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg WebP::webp WebP::webpdemux)
aui_enable_tests(aui.image)
