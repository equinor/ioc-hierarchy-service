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

target_include_directories(tag_hierarchy_test
    PRIVATE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(tag_hierarchy_test
    PRIVATE tag_hierarchy
    PRIVATE Boost::unit_test_framework
    PUBLIC ${PYTHON_LIBRARIES}
)

file(READ "unittests/test_tag_hierarchy.cpp" SOURCE_FILE_CONTENTS)
string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\)" 
       FOUND_TESTS ${SOURCE_FILE_CONTENTS})
foreach(HIT ${FOUND_TESTS})
    string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1" TEST_NAME ${HIT})
   
    add_test(NAME "tag_hierarchy_test.${TEST_NAME}" 
            COMMAND tag_hierarchy_test
            --run_test=NodesTest/${TEST_NAME} --catch_system_error=yes)
endforeach()
