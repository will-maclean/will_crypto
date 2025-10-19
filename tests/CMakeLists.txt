set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined")

find_package(CUnit QUIET)
if(NOT CUnit_FOUND)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CUNIT REQUIRED cunit)
endif()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests
  PRIVATE
    will_crypto::bigint
    will_crypto::crypto_core
    will_crypto::rng
)

if(CUnit_FOUND)
  target_link_libraries(tests PRIVATE CUnit::CUnit)
else()
  target_include_directories(tests PRIVATE ${CUNIT_INCLUDE_DIRS})
  target_link_libraries(tests PRIVATE ${CUNIT_LIBRARIES})
endif()

target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME tests COMMAND tests)
