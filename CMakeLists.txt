cmake_minimum_required(VERSION 3.7)

project(tag-hierarchy)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cppzmq/libzmq-pkg-config)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
find_package(Boost COMPONENTS serialization graph unit_test_framework REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(PythonLibs)
add_subdirectory(cpp_redis)
add_subdirectory(pybind11)
add_subdirectory(tag_hierarchy)
add_subdirectory(backup_service)

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

if (APPLE)
    set (CLIENT_INSTALL_PATH
        api/hierarchy/macos)
else (APPLE)
    set (CLIENT_INSTALL_PATH
        api/hierarchy)
endif (APPLE)

install(TARGETS tag_hierarchy_client
    LIBRARY
    DESTINATION ${CLIENT_INSTALL_PATH}
    RUNTIME
    DESTINATION ${CLIENT_INSTALL_PATH}
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

if (APPLE)
    set (BINARY_INSTALL_PATH
        bin/macos)
else (APPLE)
    set (BINARY_INSTALL_PATH
        bin)
endif (APPLE)
install(TARGETS tag-hierarchy
    RUNTIME DESTINATION ${BINARY_INSTALL_PATH})

