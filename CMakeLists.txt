cmake_minimum_required(VERSION 3.7)

project(tag-hierarchy)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cppzmq/libzmq-pkg-config)
find_package(Boost COMPONENTS graph serialization REQUIRED)
find_package(ZeroMQ REQUIRED)
add_subdirectory(pybind11)
add_subdirectory(tag_hierarchy)

pybind11_add_module(tag_hierarchy_client python/cpp/client.cpp)
target_link_libraries(tag_hierarchy_client
    PRIVATE libzmq
    PRIVATE Boost::serialization
    PUBLIC pybind11
)
target_include_directories(tag_hierarchy_client
    PRIVATE cppzmq
    PRIVATE ${CMAKE_SOURCE_DIR}
)

add_executable(tag-hierarchy main.cpp)
target_include_directories(tag-hierarchy
    PRIVATE ${CMAKE_SOURCE_DIR}
    PRIVATE cppzmq
)
target_link_libraries(tag-hierarchy
    PUBLIC python2.7
    PUBLIC tag_hierarchy
    PUBLIC pybind11
)
