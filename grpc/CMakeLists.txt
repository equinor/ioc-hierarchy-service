# Proto file
get_filename_component(hierarchy_service_proto "hierarchy_service.proto" ABSOLUTE)
get_filename_component(hierarchy_service_proto_path "${hierarchy_service_proto}" PATH)

# Generated sources
set(hierarchy_service_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/hierarchy_service.pb.cc")
set(hierarchy_service_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/hierarchy_service.pb.h")
set(hierarchy_service_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/hierarchy_service.grpc.pb.cc")
set(hierarchy_service_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/hierarchy_service.grpc.pb.h")
get_target_property(gRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
add_custom_command(
      OUTPUT "${hierarchy_service_proto_srcs}" "${hierarchy_service_proto_hdrs}" "${hierarchy_service_grpc_srcs}" "${hierarchy_service_grpc_hdrs}"
      COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hierarchy_service_proto_path}"
        --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hierarchy_service_proto}"
      DEPENDS "${hierarchy_service_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hierarchy_service_grpc_proto
add_library(hierarchy_service_grpc_proto
  ${hierarchy_service_grpc_srcs}
  ${hierarchy_service_grpc_hdrs}
  ${hierarchy_service_proto_srcs}
  ${hierarchy_service_proto_hdrs}
  "type_conversion.cpp"
)
set_target_properties(hierarchy_service_grpc_proto PROPERTIES 
    LABELS "generated"
)
set_source_files_properties(
  "type_conversion.cpp"    
  PROPERTIES LABELS "coverage"
)

target_include_directories(hierarchy_service_grpc_proto
    PRIVATE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(hierarchy_service_grpc_proto
    PUBLIC gRPC::grpc++_reflection
    PUBLIC gRPC::grpc++
    PUBLIC protobuf::libprotobuf
    PUBLIC google-cloud-cpp::api_annotations_protos
    tag_hierarchy
)

add_executable(
  grpc_server "grpc_server.cpp" 
)

target_include_directories(grpc_server
    PRIVATE ${CMAKE_SOURCE_DIR}
)
set_target_properties(grpc_server PROPERTIES 
    LABELS coverage
)

target_link_libraries(grpc_server 
    hierarchy_service_grpc_proto
    tag_hierarchy_commands
    tag_hierarchy
    pybind11::pybind11_headers
    ${PYTHON_LIBRARIES}
    gRPC::grpc++_reflection
    gRPC::grpc++
    protobuf::libprotobuf
)

install(TARGETS grpc_server
    RUNTIME DESTINATION bin
)

# Setup tests
# Unittest
add_executable(tag_hierarchy_types_test
    test_type_conversion.cpp
)

target_include_directories(tag_hierarchy_types_test
    PRIVATE ${CMAKE_SOURCE_DIR}
)
target_compile_definitions(tag_hierarchy_types_test PRIVATE BOOST_TEST_DYN_LINK)

target_link_libraries(tag_hierarchy_types_test
    PRIVATE tag_hierarchy
    PRIVATE tag_hierarchy_commands
    PRIVATE hierarchy_service_grpc_proto
    PRIVATE Boost::unit_test_framework
    PRIVATE Boost::boost
)

add_test(
  NAME "tag_hierarchy_types_test" 
  COMMAND tag_hierarchy_types_test
  --catch_system_error=yes
)

# Build python client distributable wheel
file(
  GLOB client_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/client/tag_hierarchy_grpc_client/*.py
)

add_custom_target(
  grpc_python_client
  ALL
  COMMAND pip3 install -r requirements.txt 
  COMMAND make proto_python
  COMMAND ${Python_EXECUTABLE} setup.py bdist_wheel
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/client
  SOURCES ${client_sources}
)

add_test(
  NAME grpc_wheel
  COMMAND make install_wheel
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/client
)

set_tests_properties(
  grpc_wheel
  PROPERTIES
  FIXTURES_SETUP grpc_client_install
)

# Integration test - check RPC service working
add_test(
  NAME grpc_integration_test
  COMMAND ${CMAKE_COMMAND} -E env
    CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
    ${Python_EXECUTABLE} -m unittest
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/client/tag_hierarchy_grpc_client
)

set_tests_properties(
  grpc_integration_test
  PROPERTIES
  FIXTURES_REQUIRED grpc_client_install
)
