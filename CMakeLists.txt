cmake_minimum_required (VERSION 2.6)
project (WeightFitting)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
add_executable(WeightFitting main.cpp LoadWDD.cpp)

find_package(Ceres REQUIRED)

include_directories (${CERES_INCLUDE_DIRS})
target_link_libraries (WeightFitting ${CERES_LIBRARIES})