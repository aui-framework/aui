cmake_minimum_required(VERSION 3.10)
project(base)

# the version is in the header: whoever compiles against this base sees it
add_library(base INTERFACE)
add_library(base::base ALIAS base)
target_include_directories(base INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)

install(TARGETS base EXPORT base)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT base DESTINATION cmake NAMESPACE base:: FILE baseConfig.cmake)
