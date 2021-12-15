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
  ${hierarchy_service_proto_hdrs})

target_link_libraries(hierarchy_service_grpc_proto
    gRPC::grpc++_reflection
    gRPC::grpc++
    protobuf::libprotobuf
)

add_executable(grpc_server "grpc_server.cpp")
target_include_directories(grpc_server
    PRIVATE ${CMAKE_SOURCE_DIR}
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
