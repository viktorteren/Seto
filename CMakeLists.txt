cmake_minimum_required(VERSION 3.5)
project(Progetto_SSE)

set(CMAKE_CXX_STANDARD 14)

add_executable(Progetto_SSE src/main.cpp src/Regions_generator.cpp include/Regions_generator.h include/Label_splitting_module.h src/Label_splitting_module.cpp include/TS_parser.h src/TS_parser.cpp src/Essential_region_search.cpp include/Essential_region_search.h src/Place_irredundant_pn_creation_module.cpp include/Place_irredundant_pn_creation_module.h src/Utilities.cpp include/Utilities.h src/Pre_and_post_regions_generator.cpp include/Pre_and_post_regions_generator.h src/Merging_Minimal_Preregions_module.cpp include/Merging_Minimal_Preregions_module.h)

if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")
