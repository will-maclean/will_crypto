add_executable(benchmarking)

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "*.c")
target_sources(benchmarking PRIVATE ${CORE_SOURCES})

target_include_directories(benchmarking
PRIVATE
    ${CMAKE_CURRENT_DIR}
)

target_link_libraries(benchmarking
PRIVATE
will_crypto::bigint
will_crypto::rng
will_crypto::crypto_core
)

set_target_properties(benchmarking 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    COMPILE_FLAGS "-O3"
)
