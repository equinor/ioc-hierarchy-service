## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME "Hierarchy service")
set(CTEST_SITE "localhost")
set(CTEST_BUILD_NAME "Continuous")

set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
ctest_start(Experimental)

file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CMAKE_BUILD_TYPE=Release
")

ctest_configure()
ctest_build(FLAGS -j8)
ctest_test()

set(CTEST_DROP_METHOD "https")
set(CTEST_DROP_SITE "equinor-cdash.azurewebsites.net")
set(CTEST_DROP_LOCATION "/submit.php?project=ioc-hierarchy-service")
set(CTEST_DROP_SITE_CDASH TRUE)
ctest_submit()
