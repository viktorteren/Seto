cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 14)

include(GNUInstallDirs)

add_subdirectory(pblib)
#add_subdirectory(minisat)

# include_directories(${pblib_SOURCE_DIR}/BasicPBSolver/minisat/minisat/)
#include_directories(${pblib_SOURCE_DIR}/BasicPBSolver/minisat/)
#link_directories(${pblib_SOURCE_DIR}/BasicPBSolver/minisat/)


add_executable(TS_splitter src/main.cpp src/Regions_generator.cpp include/Regions_generator.h include/Label_splitting_module.h src/Label_splitting_module.cpp include/TS_parser.h src/TS_parser.cpp src/Essential_region_search.cpp include/Essential_region_search.h src/Place_irredundant_pn_creation_module.cpp include/Place_irredundant_pn_creation_module.h src/Utilities.cpp include/Utilities.h src/Pre_and_post_regions_generator.cpp include/Pre_and_post_regions_generator.h src/Merging_Minimal_Preregions_module.cpp include/Merging_Minimal_Preregions_module.h)

target_link_libraries(TS_splitter pblib)
#target_link_libraries(TS_splitter libminisat)



if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wextra")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( TS_splitter PRIVATE -Wall -Wextra )
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( TS_splitter PRIVATE /W4 /wd4267 )
endif()

