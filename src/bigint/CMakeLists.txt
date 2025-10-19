add_library(bigint)
add_library(will_crypto::bigint ALIAS bigint)

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "*.c")
target_sources(bigint PRIVATE ${CORE_SOURCES})

target_include_directories(bigint
  PUBLIC
    ${PROJECT_INCLUDE_DIR}
  PRIVATE
    ${CMAKE_CURRENT_DIR}
)
