add_library(crypto_core)
add_library(will_crypto::crypto_core ALIAS crypto_core)

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "*.c")
target_sources(crypto_core PRIVATE ${CORE_SOURCES})

target_include_directories(crypto_core
  PUBLIC
    ${PROJECT_INCLUDE_DIR}
  PRIVATE
    ${CMAKE_CURRENT_DIR}
)

target_link_libraries(crypto_core
PUBLIC
  will_crypto::bigint
  will_crypto::rng
)

set_target_properties(crypto_core 
    PROPERTIES
    COMPILE_FLAGS "-O3"
)
