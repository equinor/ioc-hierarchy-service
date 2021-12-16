proto:
	protoc -I . --grpc_out=./build --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` hierarchy_service.proto
cpp:
	protoc -I . --cpp_out=./build hierarchy_service.proto
proto_python:
	mkdir -p proto-build
	python -m grpc_tools.protoc -I.. -p proto_build --python_out=proto_build/ --grpc_python_out=proto_build/ hierarchy_service.proto
