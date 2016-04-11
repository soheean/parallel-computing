cmake_minimum_required(VERSION 2.8)
project(ForcingMaker)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES dealer.cpp forcingMaker.h Time.hpp matrix.hpp read_data.cpp idw.cpp write_file.cpp interpolation.cpp)
add_executable(fm ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(fm pthread)
