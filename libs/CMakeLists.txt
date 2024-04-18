cmake_minimum_required(VERSION 3.5)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

set(CUDD_DIR ${CMAKE_SOURCE_DIR}/libs/cudd)
if(MSVC OR MINGW)
  if (CMAKE_CL_64)
    set(CUDD_CONFIG_DIR ${CMAKE_SOURCE_DIR}/libs/cudd-config/windows)
  else()
    set(CUDD_CONFIG_DIR ${CMAKE_SOURCE_DIR}/libs/cudd-config/windows-32)
  endif()
else()
  set(CUDD_CONFIG_DIR ${CMAKE_SOURCE_DIR}/libs/cudd-config/unix)
endif()
set(UNISTD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/unistd)
set(GETOPT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/getopt)

file(GLOB_RECURSE SOURCES
        "${CUDD_DIR}/cudd/*.c"
        "${CUDD_DIR}/util/*.c"
        "${CUDD_DIR}/st/*.c"
        "${CUDD_DIR}/epd/*.c"
        "${CUDD_DIR}/mtr/*.c"
        "${CUDD_DIR}/cplusplus/*.cc"
)


add_library(cudd STATIC ${SOURCES})
target_include_directories(cudd PRIVATE ${CUDD_DIR}/cudd ${CUDD_DIR}/util ${CUDD_DIR}/st ${CUDD_DIR}/epd ${CUDD_DIR}/mtr ${CUDD_DIR} ${CUDD_CONFIG_DIR})
if (MSVC)
  target_include_directories(cudd PRIVATE ${UNISTD_INCLUDE_DIR} ${GETOPT_INCLUDE_DIR})
endif()
#add_dependencies(cudd CUDD_DOWNLOAD)

if (MSVC)
  set(CUDD_INCLUDE_DIR ${CUDD_DIR}/cplusplus ${CUDD_DIR}/cudd ${CUDD_DIR}/util ${CUDD_CONFIG_DIR} ${UNISTD_INCLUDE_DIR} ${GETOPT_INCLUDE_DIR} PARENT_SCOPE)
else()
  set(CUDD_INCLUDE_DIR ${CUDD_DIR}/cplusplus ${CUDD_DIR}/cudd ${CUDD_DIR}/util ${CUDD_CONFIG_DIR} PARENT_SCOPE)
endif()

set(PBLIB_DIR ${CMAKE_SOURCE_DIR}/libs/pblib/pblib)

file(GLOB_RECURSE PBLIB_SOURCE_FILES
        "${PBLIB_DIR}/*.cpp"
        "${PBLIB_DIR}/encoder/*.cpp"
)
 
 add_library(pblib STATIC ${PBLIB_SOURCE_FILES})
