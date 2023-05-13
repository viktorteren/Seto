cmake_minimum_required(VERSION 3.1)
project (Seto LANGUAGES CXX C)

include(GNUInstallDirs)

add_subdirectory(libs/pblib/pblib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Grab the files
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

#target_link_libraries(${PROJECT_NAME} libminisat)
target_link_libraries(${PROJECT_NAME} pblib)
target_link_libraries(${PROJECT_NAME} python3.10)   #needed for networkx usage
target_link_libraries(${PROJECT_NAME} cudd)
target_include_directories(${PROJECT_NAME} PRIVATE ${CUDD_INCLUDE_DIR})
add_dependencies(${PROJECT_NAME} cudd)


if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options(${PROJECT_NAME} PRIVATE  -Wall -Wextra  -I/usr/include/python3.10)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4267 -I/usr/include/python3.10)
endif()