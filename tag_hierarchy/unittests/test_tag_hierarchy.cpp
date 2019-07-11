#define BOOST_TEST_MODULE example
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(tag_hierarchy_test)
BOOST_AUTO_TEST_CASE( test_test )
{
    BOOST_TEST(true);
}
BOOST_AUTO_TEST_SUITE_END()
