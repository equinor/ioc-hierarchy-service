# Object library because we use static initialization to register
# commands in the tag hierarchy service. Statically initialized
# unused variables are dropped by the linker in static libraries
add_library(tag_hierarchy_commands OBJECT
        commands/command.cpp
        commands/command.h
        commands/delete.cpp
        commands/delete.h
        commands/populategraph.cpp
        commands/populategraph.h
        commands/search.cpp
        commands/search.h
        commands/update.cpp
        commands/update.h
        commands/nodes.cpp
        commands/nodes.h
        commands/filteroptions.cpp
        commands/filteroptions.h
        utils/exceptions.h
        utils/filters.cpp
        utils/filters.h
        utils/search.h
        visitors/filteredhierarchyvisitor.h
        visitors/filteroptionsvisitor.h
        )
target_include_directories(tag_hierarchy_commands
        PRIVATE ${CMAKE_SOURCE_DIR}
        )
target_link_libraries(tag_hierarchy_commands
        PRIVATE ${PYTHON_LIBRARIES}
        PRIVATE Boost::graph
        PUBLIC Boost::serialization
        PUBLIC Python::Module
        PUBLIC pybind11::pybind11_headers
        )

add_library(tag_hierarchy STATIC
        tag_hierarchy.cpp
        tag_hierarchy.h
        visitors/searchvisitor.h)
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
    PUBLIC Python::Module
    PUBLIC pybind11::pybind11_headers
    PUBLIC libzmq)

set(UNIT_TEST_FILES
    unittests/test_tag_hierarchy.cpp
    unittests/test_filter_options.cpp
    unittests/test_delete_command.cpp
    unittests/test_load_save.cpp
    unittests/test_search.cpp
    unittests/test_update.cpp
    unittests/test_is_suppressed_field.cpp
    unittests/test_severity_field.cpp)
add_executable(tag_hierarchy_test
    unittests/fixture.cpp
    ${UNIT_TEST_FILES}
)

target_include_directories(tag_hierarchy_test
    PRIVATE ${CMAKE_SOURCE_DIR}
)
target_compile_definitions(tag_hierarchy_test PRIVATE BOOST_TEST_DYN_LINK)

target_link_libraries(tag_hierarchy_test
    PRIVATE tag_hierarchy
    PRIVATE tag_hierarchy_commands
    PRIVATE Boost::unit_test_framework
    PRIVATE Boost::boost
    PRIVATE nlohmann_json::nlohmann_json
    PUBLIC ${PYTHON_LIBRARIES}
)

file(
    COPY unittests/hierarchy_dump.json
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

foreach(UNIT_TEST_FILE ${UNIT_TEST_FILES})
file(READ "${UNIT_TEST_FILE}" SOURCE_FILE_CONTENTS)
string(REGEX MATCHALL "BOOST_.+_TEST_SUITE\\( *([A-Za-z_0-9]+).*\\)"
        TEST_SUITE_NAME ${SOURCE_FILE_CONTENTS})
set(TEST_SUITE_NAME ${CMAKE_MATCH_1})
string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\)"
       FOUND_TESTS ${SOURCE_FILE_CONTENTS})
foreach(HIT ${FOUND_TESTS})
    string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1" TEST_NAME ${HIT})
   
    add_test(NAME "tag_hierarchy_test.${TEST_NAME}" 
            COMMAND tag_hierarchy_test
            --run_test=${TEST_SUITE_NAME}/${TEST_NAME} --catch_system_error=yes)
endforeach()
endforeach()
