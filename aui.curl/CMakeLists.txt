cmake_minimum_required(VERSION 3.10)

auib_import(CURL https://github.com/aui-framework/curl/archive/refs/heads/master.zip
            ARCHIVE)

aui_module(aui.curl EXPORT aui WHOLEARCHIVE)
aui_enable_tests(aui.curl)
aui_link(aui.curl PUBLIC aui::core aui::json)
aui_link(aui.curl PRIVATE CURL::libcurl aui::crypt)
