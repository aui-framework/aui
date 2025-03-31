cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.flight_booker)
aui_link(aui.example.flight_booker PRIVATE aui::views)
