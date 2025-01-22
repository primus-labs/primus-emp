cmake_minimum_required (VERSION 3.0)
project (emp-zk)
set(NAME "emp-zk")

IF(ENABLE_WASM)
  set(CMAKE_FOLDER ${CMAKE_INSTALL_PREFIX})
  include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})

  set(EMP-TOOL_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
  set(EMP-TOOL_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libemp-tool.a ${OPENSSL_LIBRARIES})
  include_directories(${EMP-TOOL_INCLUDE_DIRS})

  set(EMP-OT_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
  set(EMP-OT_LIBRARIES "")
  include_directories(${EMP-OT_INCLUDE_DIRS})
ELSE()
  find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
  include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

  find_package(emp-ot REQUIRED)
  include_directories(${EMP-OT_INCLUDE_DIRS})
ENDIF()

set(sources
  emp-zk/emp-zk-bool/emp-zk-bool.cpp
  emp-zk/emp-zk-arith/emp-zk-arith.cpp
)
add_compile_options(-fPIC)
add_library(${NAME} ${sources})

install(DIRECTORY emp-zk DESTINATION include/)
install(FILES cmake/emp-zk-config.cmake DESTINATION cmake/)
install(TARGETS ${NAME} DESTINATION lib)

option(ENABLE_EMP_ZK_TEST "emp-zk test" ON)
IF(ENABLE_EMP_ZK_TEST)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(test)
ENDIF()
