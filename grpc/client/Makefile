proto:
	protoc -I . --grpc_out=./build --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` hierarchy_service.proto
cpp:
	protoc -I . --cpp_out=./build hierarchy_service.proto
proto_python:
	mkdir -p proto_build
	python3 -m grpc_tools.protoc -I.. --python_out=tag_hierarchy_grpc_client/ --grpc_python_out=tag_hierarchy_grpc_client/ ../hierarchy_service.proto
