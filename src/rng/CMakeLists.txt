add_library(rng)
add_library(will_crypto::rng ALIAS rng)

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "*.c")
target_sources(rng PRIVATE ${CORE_SOURCES})

target_include_directories(rng
  PUBLIC
    ${PROJECT_INCLUDE_DIR}
  PRIVATE
    ${CMAKE_CURRENT_DIR}
)

target_link_libraries(rng
PUBLIC
  will_crypto::bigint
)

set_target_properties(rng 
    PROPERTIES
    COMPILE_FLAGS "-O3"
)
