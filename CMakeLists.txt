cmake_minimum_required(VERSION 3.7)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

project(tag-hierarchy)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)


find_package(nlohmann_json CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

enable_testing()


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cppzmq/libzmq-pkg-config)

find_package(Boost COMPONENTS serialization graph iostreams unit_test_framework REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11)

set(BUILD_TESTING OFF)
set(JSON_BuildTests OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_TESTING ON)
set(CMAKE_DIRECTORY_LABELS tag_hierarchy)

add_subdirectory(tag_hierarchy)
add_subdirectory(backup_service)
add_subdirectory(grpc)

pybind11_add_module(tag_hierarchy_client python/cpp/client.cpp)

target_link_libraries(tag_hierarchy_client
    PRIVATE Boost::serialization
    PRIVATE libzmq
    PRIVATE Python::Module
    PUBLIC pybind11::pybind11_headers
)
target_include_directories(tag_hierarchy_client
    PRIVATE cppzmq
    PRIVATE ${CMAKE_SOURCE_DIR}
)

option(USE_LOCAL_PYTHON "Build for local python in /usr/local" ON)
if(USE_LOCAL_PYTHON)
  set(PYTHON_LIBRARY_LOCATION /usr/local/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages)
else()
  set(PYTHON_LIBRARY_LOCATION ${Python_SITEARCH})
endif()

install(TARGETS tag_hierarchy_client
     LIBRARY
     DESTINATION ${PYTHON_LIBRARY_LOCATION}
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
    PUBLIC pybind11::pybind11_headers
    PUBLIC ${PYTHON_LIBRARIES}
)

install(TARGETS tag-hierarchy
    RUNTIME DESTINATION bin
)

set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)

set(CPACK_PACKAGE_NAME "tag-hierarchy")
set(CPACK_PACKAGE_VENDOR "Equinor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "tag-hierarchy - handling tag hierarchies since 2019")
set(CPACK_PACKAGE_VERSION "2.0.0-alpha1")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0-alpha1")
set(CPACK_SYSTEM_NAME "x86-64")
set(CPACK_INSTALL_PREFIX /usr)
set(CPACK_SET_DESTDIR "ON")

# Debian settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Equinor - IOC")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# RPM settings
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
  "/usr/lib64/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}"
  "${Python_SITEARCH}"
)
set(CPACK_RPM_PACKAGE_RELOCATABLE 0)

include(CPack)
