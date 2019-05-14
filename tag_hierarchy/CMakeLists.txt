add_library(tag_hierarchy tag_hierarchy.cpp tag_hierarchy.h)

target_include_directories(tag_hierarchy
    PRIVATE ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/cppzmq
)
target_link_libraries(tag_hierarchy
    PUBLIC Boost::serialization
    PUBLIC pybind11
    PUBLIC libzmq)
