cmake_minimum_required(VERSION 3.9)
project(Progetto_SSE)

set(CMAKE_CXX_STANDARD 14)

add_executable(Progetto_SSE main.cpp Minimal_pre_region_generator.cpp Minimal_pre_region_generator.h Label_splitting_module.h Label_splitting_module.cpp TS_parser.h TS_parser.cpp essential_region_search.cpp essential_region_search.h)

include_directories( ~/boost_1_66_0 )

set(BOOST_ROOT "~/boost_1_66_0")
set(BOOSTROOT "~/boost_1_66_0")
find_package(Boost 1.60.0)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

set(SOURCE_FILES chat_server.cpp)