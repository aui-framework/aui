# AUI configuration file

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include_directories(${SELF_DIR}/include)

include(${SELF_DIR}/cmake/AUI.cmake)
include(${SELF_DIR}/cmake/AUI.Build.cmake)