cmake_minimum_required(VERSION 3.10)
project(WillCrypto C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/tests.c
)

add_executable(main
    ${SRC_FILES}
    ${PROJECT_SOURCE_DIR}/src/main.c
)

add_executable(tests
    ${SRC_FILES}
    ${PROJECT_SOURCE_DIR}/src/tests.c
)

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined")

set_target_properties(main tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    COMPILE_FLAGS "-g -Wall -Wextra -pedantic"
)

