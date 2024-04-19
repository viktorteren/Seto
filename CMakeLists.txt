cmake_minimum_required(VERSION 3.1)
project (Seto LANGUAGES CXX C)

include(GNUInstallDirs)

add_library(libminisat STATIC
        # Impl files
        libs/minisat/core/Solver.cc
        libs/minisat/core/SolverTypes.cc
        libs/minisat/utils/Options.cc
        # Header files for IDEs
        libs/minisat/core/Dimacs.h
        libs/minisat/core/Solver.h
        libs/minisat/core/SolverTypes.h
        libs/minisat/mtl/Alg.h
        libs/minisat/mtl/Alloc.h
        libs/minisat/mtl/Heap.h
        libs/minisat/mtl/IntTypes.h
        libs/minisat/mtl/Map.h
        libs/minisat/mtl/Queue.h
        libs/minisat/mtl/Sort.h
        libs/minisat/mtl/Vec.h
        libs/minisat/mtl/XAlloc.h
        libs/minisat/utils/Options.h)

target_include_directories(libminisat
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Grab the files
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} libminisat)
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