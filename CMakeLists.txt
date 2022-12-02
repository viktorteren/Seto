#########################
# Important note: Minisat does not properly annotate its API with
# dllimport/dllexport, so the library build is forced to be static.
#########################
cmake_minimum_required(VERSION 3.5)
project (TS_splitter LANGUAGES CXX C)

include(GNUInstallDirs)

add_subdirectory(pblib)

add_library(libminisat STATIC
        # Impl files
        minisat/core/Solver.cc
        minisat/core/SolverTypes.cc
        minisat/utils/Options.cc
        minisat/utils/System.cc
        minisat/simp/SimpSolver.cc
        # Header files for IDEs
        minisat/core/Dimacs.h
        minisat/core/Solver.h
        minisat/core/SolverTypes.h
        minisat/mtl/Alg.h
        minisat/mtl/Alloc.h
        minisat/mtl/Heap.h
        minisat/mtl/IntTypes.h
        minisat/mtl/Map.h
        minisat/mtl/Queue.h
        minisat/mtl/Sort.h
        minisat/mtl/Vec.h
        minisat/mtl/XAlloc.h
        minisat/utils/Options.h
        minisat/utils/ParseUtils.h
        minisat/utils/System.h
        minisat/simp/SimpSolver.h
        include/PN_decomposition.h
        include/GreedyRemoval.h)

target_include_directories(libminisat
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Grab the files
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} src/BDD_encoder.cpp include/BDD_encoder.h)
# adding TS decomposition code

target_link_libraries(${PROJECT_NAME} libminisat)
target_link_libraries(${PROJECT_NAME} pblib)
target_link_libraries(${PROJECT_NAME} python3.10)
target_link_libraries(${PROJECT_NAME} cudd)
target_include_directories(${PROJECT_NAME} PRIVATE ${CUDD_INCLUDE_DIR})
add_dependencies(${PROJECT_NAME} cudd)


if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options(${PROJECT_NAME} PRIVATE  -Wall -Wextra  -I/usr/include/python3.10 )
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4267 -I/usr/include/python3.10 )
endif()