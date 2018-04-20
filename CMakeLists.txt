cmake_minimum_required(VERSION 3.9)
project(Progetto_SSE)

set(CMAKE_CXX_STANDARD 14)

add_executable(Progetto_SSE main.cpp Regions_generator.cpp Regions_generator.h Label_splitting_module.h Label_splitting_module.cpp TS_parser.h TS_parser.cpp Essential_region_search.cpp Essential_region_search.h Irredundant_sets_creation_module.cpp Irredundant_sets_creation_module.h Utilities.cpp Utilities.h Pre_regions_generator.cpp Pre_regions_generator.h)

set(SOURCE_FILES chat_server.cpp)