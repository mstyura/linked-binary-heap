cmake_minimum_required(VERSION 3.14)

project(linked_binary_heap
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O3 -DNDEBUG")


option(sanitize_address "Compile with address sanitizer" 0)

option(sanitize_memory "Compile with memory sanitizer" 0)

if (sanitize_address AND sanitize_memory)
	message(FATAL_ERROR "Memory and Address sanitizers can not both be enabled")
endif ()

if (CMAKE_C_COMPILER_ID MATCHES "MSVC" OR
    CMAKE_C_SIMULATE_ID MATCHES "MSVC")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")

elseif (CMAKE_C_COMPILER_ID MATCHES "Clang" OR
        CMAKE_C_COMPILER_ID MATCHES "AppleClang" OR
        CMAKE_C_COMPILER_ID MATCHES "GNU")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")

    if (sanitize_address)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined,signed-integer-overflow -fno-omit-frame-pointer -fno-sanitize-recover=all -fsanitize-address-use-after-scope")
    endif ()

    if (sanitize_memory)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -fsanitize-memory-track-origins")
    endif ()
endif()


add_library(linked_binary_heap_library
    STATIC
        src/linked_binary_heap.c
)

target_include_directories(linked_binary_heap_library
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

add_executable(linked_binary_heap_tests
    src/linked_binary_heap_tests.c
)

target_link_libraries(linked_binary_heap_tests
    PRIVATE
        linked_binary_heap_library
)