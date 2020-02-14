## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME "Hierarchy service")
set(CTEST_SITE "localhost")
set(CTEST_BUILD_NAME "Coverage")
set(CTEST_COVERAGE_COMMAND "gcov")
set(CTEST_COVERAGE_EXTRA_FLAGS "-l -p")

set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
ctest_start(Experimental)

file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CMAKE_CXX_FLAGS=-g -O0 -fprofile-arcs -ftest-coverage -fno-omit-frame-pointer
CMAKE_LDFLAGS=-fprofile-arcs -ftest-coverage
")

ctest_configure()
ctest_build(FLAGS -j8)
ctest_test(EXCLUDE ^opencensus)
ctest_coverage(EXCLUDE ^opencensus)

set(CTEST_DROP_METHOD "https")
set(CTEST_DROP_SITE "equinor-cdash.azurewebsites.net")
set(CTEST_DROP_LOCATION "/submit.php?project=ioc-hierarchy-service")
set(CTEST_DROP_SITE_CDASH TRUE)
ctest_submit()
