add_executable(will_crypto)

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "*.c")
target_sources(will_crypto PRIVATE ${CORE_SOURCES})

target_include_directories(will_crypto
  PRIVATE
    ${CMAKE_CURRENT_DIR}
)

target_link_libraries(will_crypto
PRIVATE
will_crypto::bigint
will_crypto::rng
will_crypto::crypto_core
)

set_target_properties(will_crypto 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    COMPILE_FLAGS "-O3"
)