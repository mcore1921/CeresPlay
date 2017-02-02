cmake_minimum_required (VERSION 2.6)
project (HelloWorldCeres)
add_executable(HelloWorldCeres main.cpp)

find_package(Ceres REQUIRED)

include_directories (${CERES_INCLUDE_DIRS})
target_link_libraries (HelloWorldCeres ${CERES_LIBRARIES})