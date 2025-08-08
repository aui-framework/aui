include(CheckFunctionExists)

check_function_exists(fopen64 HAVE_FOPEN64)

target_sources(aui.core PRIVATE
    minizip/ioapi.c
    minizip/unzip.c
    minizip/zip.c
)
target_include_directories(aui.core PRIVATE ".")

target_compile_definitions(aui.core
    PUBLIC  $<$<BOOL:NOT:${HAVE_FOPEN64}>:USE_FILE32API=1>
    PRIVATE $<$<BOOL:${MSVC}>:_CRT_SECURE_NO_WARNINGS>
)
