proto_python:
	mkdir -p proto_build
	python3 -m grpc_tools.protoc -I.. --python_out=tag_hierarchy_grpc_client/ --grpc_python_out=tag_hierarchy_grpc_client/ ../hierarchy_service.proto

install_wheel:
	pip3 install $$(find . -type f -name "*.whl")
