cmake_minimum_required(VERSION 3.10)
project(WillCrypto C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_CLI "Build will_crypto cli" ON)
option(BUILD_TESTS "Build will_crypto cli" ON)
option(BUILD_SHARED_LIBS "Build will_crypto cli" ON)

set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(src/bigint)
add_subdirectory(src/rng)
add_subdirectory(src/crypto_core)

if(BUILD_CLI)
add_subdirectory(src/cli)
endif()

if(BUILD_TESTS)
add_subdirectory(tests)
endif()
