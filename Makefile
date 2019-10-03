nightly:
	ctest -S ctest_scripts/CTestCoverage.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build
	ctest -S ctest_scripts/CTestASAN.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build
	ctest -S ctest_scripts/CTestMSAN.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build
	ctest -S ctest_scripts/CTestTSAN.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build

continuous:
	rm -rf *.o ../ioc-hierarchy-service-docker-build
	ctest -S ctest_scripts/CTestContinuous.cmake -DCTEST_SOURCE_DIRECTORY=. -DCTEST_BINARY_DIRECTORY=../ioc-hierarchy-service-docker-build

