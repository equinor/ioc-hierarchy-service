## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME "Hierarchy service")
set(CTEST_SITE "localhost")
set(CTEST_BUILD_NAME "AddressSanitizer")
 
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_MEMORYCHECK_TYPE "AddressSanitizer")

ctest_start(Experimental)

file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CMAKE_CXX_COMPILER=g++
CMAKE_CXX_FLAGS=-g -O1 -fsanitize=address -fno-omit-frame-pointer
CMAKE_LDFLAGS= -fsanitize=address
")

ctest_configure()
ctest_build(FLAGS -j8)
ctest_test(
  EXCLUDE integration
  RETURN_VALUE test_result
)
ctest_memcheck(
  EXCLUDE integration
  RETURN_VALUE memcheck_result
)

set(CTEST_DROP_METHOD "https")
set(CTEST_DROP_SITE "s039-ioc-cdash.azurewebsites.net")
set(CTEST_DROP_LOCATION "/submit.php?project=ioc-hierarchy-service")
set(CTEST_DROP_SITE_CDASH TRUE)
ctest_submit()

if (test_result OR memcheck_result)
  message(FATAL_ERROR "Tests failed")
endif ()
