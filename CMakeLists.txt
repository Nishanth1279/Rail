cmake_minimum_required(VERSION 3.16)
project(RailwayReservationSystem LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB RAILWAY_SOURCES CONFIGURE_DEPENDS "*.c")
add_executable(railway ${RAILWAY_SOURCES})

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(railway PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wformat=2
    )
    target_link_libraries(railway PRIVATE m)
endif()
