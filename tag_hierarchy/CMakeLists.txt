# Object library because we use static initialization to register
# commands in the tag hierarchy service. Statically initialized
# unused variables are dropped by the linker in static libraries
add_library(tag_hierarchy_commands OBJECT
        commands/command.cpp
        commands/populategraph.cpp
        commands/nodes.cpp
        commands/kpinodes.cpp
        commands/filteroptions.cpp
        commands/create_kpitree.cpp
)
target_include_directories(tag_hierarchy_commands
        PRIVATE ${CMAKE_SOURCE_DIR}
        )
target_link_libraries(tag_hierarchy_commands
        PUBLIC opencensus-cpp::trace
        PUBLIC opencensus-cpp::context
        PRIVATE ${PYTHON_LIBRARIES}
        PRIVATE Boost::graph
        PUBLIC Boost::serialization
        PRIVATE pybind11
        )

add_library(tag_hierarchy STATIC
        tag_hierarchy.cpp
        tag_hierarchy.h
        visitors/createkpitreevisitor.h)
# Compile with -fpic so we can link a shared library into it
set_property(TARGET tag_hierarchy PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(tag_hierarchy
    PRIVATE ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/cppzmq
)

find_package(PythonLibs)

target_link_libraries(tag_hierarchy
    PUBLIC tag_hierarchy_commands
    PUBLIC ${PYTHON_LIBRARIES}
    PRIVATE Boost::iostreams
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
    PRIVATE tag_hierarchy_commands
    PUBLIC opencensus-cpp::trace
    PUBLIC opencensus-cpp::exporters_trace_stdout
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
