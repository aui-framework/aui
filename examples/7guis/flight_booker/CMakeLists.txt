cmake_minimum_required(VERSION 3.10)

auib_import(ctre https://github.com/hanickadot/compile-time-regular-expressions
            VERSION v3.9.0)

aui_executable(aui.example.flight_booker)
aui_link(aui.example.flight_booker PRIVATE aui::views ctre::ctre)
