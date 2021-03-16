nightly:
	ctest -S ctest_scripts/CTestCoverage.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build
	ctest -S ctest_scripts/CTestASAN.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build
	ctest -S ctest_scripts/CTestMSAN.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build
# Disabling thread sanitizer build for now as the build is mysteriously failing in the abseil code
#	ctest -S ctest_scripts/CTestTSAN.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build

continuous:
	rm -rf *.o ../ioc-hierarchy-service-docker-build
	ctest -S ctest_scripts/CTestContinuous.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build

build_package:
	docker build -t tag-hierarchy ./
	docker create --name tag-hierarchy-service tag-hierarchy
	docker cp tag-hierarchy-service:/usr/src/app/ioc-hierarchy-service-docker-build/tag-hierarchy-1.2.0-x86-64.deb .
	docker rm tag-hierarchy-service

build_debug_package:
	docker build -t tag-hierarchy ./ --build-arg CMAKE_BUILD_TYPE=Debug
	docker create --name tag-hierarchy-service tag-hierarchy
	docker cp tag-hierarchy-service:/usr/src/app/ioc-hierarchy-service-docker-build/tag-hierarchy-1.2.0-x86-64.deb .
	docker rm tag-hierarchy-service	
