cmake_minimum_required(VERSION 3.7)

project(tag-hierarchy)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cppzmq/libzmq-pkg-config)
find_package(Boost COMPONENTS serialization graph REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(PythonLibs)
add_subdirectory(pybind11)
add_subdirectory(tag_hierarchy)

pybind11_add_module(tag_hierarchy_client python/cpp/client.cpp)
target_link_libraries(tag_hierarchy_client
    PRIVATE Boost::serialization
    PRIVATE libzmq
    PUBLIC pybind11
)
target_include_directories(tag_hierarchy_client
    PRIVATE cppzmq
    PRIVATE ${CMAKE_SOURCE_DIR}
)
install(TARGETS tag_hierarchy_client
    LIBRARY
    DESTINATION api/hierarchy
    RUNTIME
    DESTINATION api/hierarchy
)

add_executable(tag-hierarchy main.cpp)
target_include_directories(tag-hierarchy
    PRIVATE ${CMAKE_SOURCE_DIR}
    PRIVATE cppzmq
)
target_link_libraries(tag-hierarchy
    PUBLIC tag_hierarchy
    PUBLIC pybind11
    PUBLIC ${PYTHON_LIBRARIES}
)
install(TARGETS tag-hierarchy
    RUNTIME DESTINATION bin/)
