cmake_minimum_required (VERSION 3.0)
project (emp-ot)
set(NAME "emp-ot")

IF(ENABLE_WASM)
  set(CMAKE_FOLDER ${CMAKE_PREFIX_PATH})
  include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})

  set(EMP-TOOL_INCLUDE_DIRS ${CMAKE_PREFIX_PATH}/include)
  set(EMP-TOOL_LIBRARIES ${CMAKE_PREFIX_PATH}/lib/libemp-tool.a ${OPENSSL_LIBRARIES})
  include_directories(${EMP-TOOL_INCLUDE_DIRS})

  set(NO_WARN_FLAGS "-Wno-unused-function -Wno-unused-command-line-argument")
  set(CCFLAGS "-msimd128 -msse -msse2 -msse4.1 -msse4.2 -s ASSERTIONS=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NO_WARN_FLAGS} ${CCFLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NO_WARN_FLAGS} ${CCFLAGS}")
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
