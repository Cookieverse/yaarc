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

if (YAARC_USE_BOOST_ASIO)
    target_compile_definitions(yaarc PUBLIC YAARC_USE_BOOST_ASIO)

    set(Boost_USE_STATIC_LIBS TRUE)
    # cant find_package boost asio apparently: https://stackoverflow.com/a/35379605
    find_package(Boost REQUIRED COMPONENTS system thread)
    target_link_libraries(yaarc PRIVATE  Boost::system Boost::thread)
else()
    target_link_libraries(yaarc PRIVATE asio asio::asio)
    find_path(ASIO_INCLUDE_DIR asio.hpp)
    include_directories(${ASIO_INCLUDE_DIR})
endif()

target_link_libraries(yaarc PRIVATE fmt::fmt-header-only)
include_directories("${PROJECT_SOURCE_DIR}/include/")
ENABLE_TESTING()
ADD_SUBDIRECTORY(tests)