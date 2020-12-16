cmake_minimum_required(VERSION 3.9)
project(yaarc)

set(CMAKE_CXX_STANDARD 14)

SET(YaarcBuildType STATIC CACHE STRING "Build type, SHARED or STATIC")
set(YaarcFiles include/yaarc.hpp)
include_directories(${PROJECT_SOURCE_DIR}/include/)
add_library(yaarc ${AsioBuildType} ${AsioFiles})