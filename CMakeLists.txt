cmake_minimum_required(VERSION 3.10)
project (Seto LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

option(SETO_USE_CADICAL "Build with CaDiCaL SAT solver" ON)
option(SETO_NATIVE_OPT "Enable -march=native in Release builds" ON)

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
target_link_libraries(${PROJECT_NAME} cudd)
target_include_directories(${PROJECT_NAME} PRIVATE ${CUDD_INCLUDE_DIR})
add_dependencies(${PROJECT_NAME} cudd)

if(SETO_USE_CADICAL AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/cadical/src/cadical.hpp")
    file(GLOB CADICAL_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/cadical/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/cadical/src/*.c"
    )
    list(FILTER CADICAL_SOURCES EXCLUDE REGEX ".*/(cadical|mobical)\\.cpp$")
    add_library(cadical STATIC ${CADICAL_SOURCES})
    target_compile_definitions(cadical PRIVATE NBUILD QUIET NUNLOCKED)
    if (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(cadical PRIVATE -mno-ms-bitfields)
    endif()
    target_include_directories(cadical PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libs/cadical/src")
    target_link_libraries(${PROJECT_NAME} cadical)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SETO_WITH_CADICAL)
endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    if (SETO_NATIVE_OPT)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3 -march=native>)
    endif()
endif()

find_program(NVCC_EXECUTABLE nvcc)
if(NVCC_EXECUTABLE)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mis_kernels.ptx"
        COMMAND "${NVCC_EXECUTABLE}" -ptx -O3 -arch=compute_75
                "${CMAKE_CURRENT_SOURCE_DIR}/src/mis_kernels.cu"
                -o "${CMAKE_CURRENT_BINARY_DIR}/mis_kernels.ptx"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/mis_kernels.cu"
        COMMENT "Compiling CUDA PTX kernels for MIS"
    )
    add_custom_target(mis_ptx ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/mis_kernels.ptx")
    add_dependencies(${PROJECT_NAME} mis_ptx)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4267)
endif()