if (AUI_PLATFORM_WIN)
    if ($ENV{CI})
        # on windows on github ci there's old msvc compiler which literally crashes (C1001 Internal compiler error)
        # these examples are used in code snippets, so i don't want to screw them up just because shitty microsoft
        # compiler can't accept perfectly valid c++ code.
        return()
    endif()
endif()

file(GLOB items "*")
foreach (item ${items})
    if (NOT EXISTS "${item}/CMakeLists.txt")
        continue()
    endif()
    add_subdirectory(${item})
endforeach ()
