cmake_minimum_required(VERSION 3.9)
project(yaarc)

set(CMAKE_CXX_STANDARD 17)

SET(YaarcBuildType STATIC CACHE STRING "Build type, SHARED or STATIC")
set(YaarcFiles yaarc.cpp)
add_library(yaarc ${YaarcBuildType} ${YaarcFiles})
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_compile_options("/wd4068") # no warning spam about unknown pragmas
endif ()
find_package(Asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(yaarc INTERFACE <threads>)
target_link_libraries(yaarc PRIVATE asio asio::asio)
target_link_libraries(yaarc PRIVATE fmt::fmt-header-only)
find_path(ASIO_INCLUDE_DIR asio.hpp)
include_directories(${ASIO_INCLUDE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/include/")
ENABLE_TESTING()
ADD_SUBDIRECTORY(tests)