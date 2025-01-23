cmake_minimum_required (VERSION 3.0)
project (emp-ot)
set(NAME "emp-ot")

IF(ENABLE_WASM OR ENABLE_ANDROID)
  set(CMAKE_FOLDER ${CMAKE_INSTALL_PREFIX})
  include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})

  set(EMP-TOOL_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
  set(EMP-TOOL_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libemp-tool.a ${OPENSSL_LIBRARIES})
  include_directories(${EMP-TOOL_INCLUDE_DIRS})
ELSE()
  find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
  include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

  find_package(emp-tool REQUIRED)
  include_directories(${EMP-TOOL_INCLUDE_DIRS})
ENDIF()

# Installation
install(FILES cmake/emp-ot-config.cmake DESTINATION cmake/)
install(DIRECTORY emp-ot DESTINATION include/)

option(ENABLE_EMP_OT_TEST "emp-ot test" ON)
IF(ENABLE_EMP_OT_TEST)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(test)
ENDIF()
