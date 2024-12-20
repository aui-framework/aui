cmake_minimum_required(VERSION 3.10)

auib_import(CURL https://github.com/aui-framework/curl/archive/refs/heads/master.zip
            ARCHIVE
            PRECOMPILED_URL_PREFIX https://github.com/aui-framework/curl/releases/download/refs%2Fheads%2Fmaster)

aui_module(aui.curl EXPORT aui)
aui_enable_tests(aui.curl)
aui_link(aui.curl PUBLIC aui::core aui::json)
aui_link(aui.curl PRIVATE CURL::libcurl aui::crypt)
aui_compile_assets(aui.curl)