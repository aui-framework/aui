cmake_minimum_required(VERSION 3.10)

auib_import(ctre https://github.com/hanickadot/compile-time-regular-expressions
            VERSION 6225211806c48230e5d17a1e555ef69e7325051c
            CMAKE_ARGS -DCTRE_BUILD_TESTS=OFF)

aui_executable(aui.example.flight_booker)
aui_link(aui.example.flight_booker PRIVATE aui::views ctre::ctre)
