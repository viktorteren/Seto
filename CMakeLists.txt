#########################
# Important note: Minisat does not properly annotate its API with
# dllimport/dllexport, so the library build is forced to be static.
#########################
cmake_minimum_required(VERSION 3.5)
project(TS_splitter LANGUAGES CXX)

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
        include/FCPN_decomposition.h
        include/GreedyRemoval.h)

target_include_directories(libminisat
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

# adding TS decomposition code
add_executable(TS_splitter
        src/main.cpp
        src/Regions_generator.cpp
        include/Regions_generator.h
        include/Label_splitting_module.h
        src/Label_splitting_module.cpp
        include/TS_parser.h
        src/TS_parser.cpp
        src/Essential_region_search.cpp
        include/Essential_region_search.h
        src/Place_irredundant_pn_creation_module.cpp
        include/Place_irredundant_pn_creation_module.h
        src/Utilities.cpp include/Utilities.h
        src/Pre_and_post_regions_generator.cpp
        include/Pre_and_post_regions_generator.h
        src/Merging_Minimal_Preregions_module.cpp
        include/Merging_Minimal_Preregions_module.h
        src/Merge.cpp
        include/Merge.h
        src/FCPN_decomposition.cpp
        src/k_FCPN_decomposition.cpp
        include/k_FCPN_decomposition.h
        src/GreedyRemoval.cpp include/FCPN_Merge.h src/FCPN_Merge.cpp include/FCPN_composition.h src/FCPN_composition.cpp)
target_link_libraries(TS_splitter libminisat)
target_link_libraries(TS_splitter pblib)
target_link_libraries(TS_splitter python2.7)

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( TS_splitter PRIVATE  -Wall -Wextra  -I/usr/include/python2.7 )
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( TS_splitter PRIVATE /W4 /wd4267 -I/usr/include/python2.7 )
endif()













