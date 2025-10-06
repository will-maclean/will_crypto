cmake_minimum_required(VERSION 3.10)
project(WillCrypto C)

# Specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directory for headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Collect all .c files from src (excluding main.c and tests.c)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/tests.c
    ${PROJECT_SOURCE_DIR}/src/rsa.c
)

# Create main executable
add_executable(main
    ${SRC_FILES}
    ${PROJECT_SOURCE_DIR}/src/main.c
)

# Create tests executable
add_executable(tests
    ${SRC_FILES}
    ${PROJECT_SOURCE_DIR}/src/tests.c
)

# (Optional) Set output directories
set_target_properties(main tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    COMPILE_FLAGS "-g -Wall -Wextra -pedantic"
)

