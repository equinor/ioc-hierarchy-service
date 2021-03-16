#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE( SearchTest, Fixture );
    BOOST_AUTO_TEST_CASE( test_search_regex )
    {
        const auto search_keys = std::vector<std::string>{"name", "tag"};
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("search")},
                         {std::string("search_term"), std::string("Level3.[1-2].*output")},
                         {std::string("search_keys"), search_keys},
                         {std::string("search_algorithm"), std::string("regex")},
                         {std::string("max_results"), 200000},
                 }}
        );
        auto response = TagHierarchy::Handle(query);
        BOOST_TEST(response.size() == 22);
    };

    BOOST_AUTO_TEST_CASE( test_search_boyer_moore )
    {
        const auto search_keys = std::vector<std::string>{"name", "tag"};
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("search")},
                         {std::string("search_term"), std::string("output")},
                         {std::string("search_keys"), search_keys},
                         {std::string("search_algorithm"), std::string("boyer-moore")},
                         {std::string("max_results"), 20000},
                 }}
        );
        auto response = TagHierarchy::Handle(query);
        BOOST_TEST(response.size() == 34);
    }

BOOST_AUTO_TEST_SUITE_END()
