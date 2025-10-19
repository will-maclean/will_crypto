set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,undefined")

function(add_c_test NAME)
  add_executable(${NAME} ${ARGN})
  target_link_libraries(${NAME} 
    PRIVATE
      will_crypto::bigint 
      will_crypto::crypto_core 
      will_crypto::rng
  )
  target_include_directories(${NAME} PRIVATE ${PROJECT_INCLUDE_DIR})
  add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.c")
foreach(TEST_SRC ${TEST_SOURCES})
  get_filename_component(TNAME "${TEST_SRC}" NAME_WE)
  add_c_test(${TNAME} ${TEST_SRC})
endforeach()

