cmake_minimum_required(VERSION 3.27)
project(hi-c LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

# NASM Setup
enable_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER_LOADED)
    set(CAN_USE_ASSEMBLER TRUE)
endif()

add_executable(hi-c
        src/main.c
        src/hash/hash.c
        src/wdex/wdex.c
        src/util/console.c
        src/calc/calc.cpp
        src/util/fis.c
        src/find/find.c
        src/path/path.c
        src/util/io.c
        src/size/size.c
        src/stat/stat.c
        src/util/secure/sha256.c
        src/util/secure/fx.c
        src/sync/sync.c
        src/sync/cache.c
        src/sync/flist.c
        src/sync/diff.c
        src/open/open.c
)

set_target_properties(hi-c PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
)
