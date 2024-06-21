cmake_minimum_required(VERSION 3.27)
project(hi-c LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

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
)

set_target_properties(hi-c PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
)
