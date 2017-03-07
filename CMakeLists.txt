cmake_minimum_required (VERSION 2.6)
project (WeightFitting)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
#SET(CMAKE_CXX_FLAGS "-std=c++0x -DDEBUG_OUTPUT -g")
#SET(CMAKE_CXX_FLAGS "-std=c++0x -g")
add_executable(WeightFitting main.cpp WDDSet.cpp Solution.cpp LinearSolution.cpp CalorieFitSolution.cpp HBSolution.cpp)

find_package(Ceres REQUIRED)

include_directories (${CERES_INCLUDE_DIRS})
target_link_libraries (WeightFitting ${CERES_LIBRARIES})