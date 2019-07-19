## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME "Hierarchy service")
set(CTEST_SITE "localhost")
set(CTEST_BUILD_NAME "UndefinedBehaviorSanitizer")
 
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_MEMORYCHECK_TYPE "UndefinedBehaviorSanitizer")

ctest_start(Experimental)

file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CMAKE_CXX_COMPILER=g++
CMAKE_CXX_FLAGS=-g -O1 -fsanitize=undefined -fno-omit-frame-pointer
")

ctest_configure()
ctest_build(FLAGS -j8)
ctest_test()
ctest_memcheck()

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "host.docker.internal:8080")
set(CTEST_DROP_LOCATION "/submit.php?project=Hierarchy+service")
set(CTEST_DROP_SITE_CDASH TRUE)
ctest_submit()
