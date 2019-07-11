add_library(tag_hierarchy STATIC
    tag_hierarchy.cpp
    tag_hierarchy.h
)

target_include_directories(tag_hierarchy
    PRIVATE ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/cppzmq
)

target_link_libraries(tag_hierarchy
    PUBLIC Boost::serialization
    PUBLIC pybind11
    PUBLIC libzmq)

add_executable(tag_hierarchy_test
    unittests/test_tag_hierarchy.cpp
)

add_test(NAME tag_hierarchy_test COMMAND tag_hierarchy_test)
target_link_libraries(tag_hierarchy_test
    PRIVATE tag_hierarchy
    PRIVATE Boost::unit_test_framework
)
