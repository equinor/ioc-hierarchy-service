cmake_minimum_required(VERSION 3.7)

project(tag-hierarchy)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cppzmq/libzmq-pkg-config)

find_package(Boost COMPONENTS serialization graph iostreams unit_test_framework REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(Python COMPONENTS Interpreter Development)

add_subdirectory(cpp_redis)
add_subdirectory(pybind11)
set(BUILD_TESTING OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(opencensus-cpp)
set(BUILD_TESTING ON)
set(CMAKE_DIRECTORY_LABELS tag_hierarchy)
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
install(TARGETS tag_hierarchy_client
     LIBRARY
     DESTINATION ${Python_SITEARCH}
     RUNTIME
     DESTINATION bin
)

add_executable(tag-hierarchy
        main.cpp
        models/models.h
        )
target_include_directories(tag-hierarchy
    PRIVATE ${CMAKE_SOURCE_DIR}
    PRIVATE cppzmq
)
target_link_libraries(tag-hierarchy
    PRIVATE tag_hierarchy_commands
    PUBLIC tag_hierarchy
    PRIVATE Boost::serialization
    PUBLIC opencensus-cpp::trace
    PUBLIC opencensus-cpp::exporters_trace_stdout
    PUBLIC pybind11
    PUBLIC ${PYTHON_LIBRARIES}
)

install(TARGETS tag-hierarchy
    RUNTIME DESTINATION bin
)

set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)

set(CPACK_PACKAGE_NAME "tag-hierarchy")
set(CPACK_PACKAGE_VENDOR "Equinor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "tag-hierarchy - handling tag hierarchies since 2019")
set(CPACK_PACKAGE_VERSION "1.1.3")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "3")
SET(CPACK_SYSTEM_NAME "x86-64")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Equinor - IOC")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_SET_DESTDIR "ON")
set(CPACK_INSTALL_PREFIX /usr)

include(CPack)
