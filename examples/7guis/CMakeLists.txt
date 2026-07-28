if (AUI_PLATFORM_WIN)
    if ($ENV{CI})
        # on windows on github ci there's old msvc compiler which literally crashes (C1001 Internal compiler error)
        # these examples are used in code snippets, so i don't want to screw them up just because shitty microsoft
        # compiler can't accept perfectly valid c++ code.
        return()
    endif()
endif()

add_subdirectory(counter)
add_subdirectory(temperature_converter)
add_subdirectory(flight_booker)
add_subdirectory(timer)
add_subdirectory(crud)
add_subdirectory(circle_drawer)
add_subdirectory(cells)
