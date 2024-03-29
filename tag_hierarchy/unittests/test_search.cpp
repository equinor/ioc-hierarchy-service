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
        BOOST_TEST(response.size() == 24);
    };

    BOOST_AUTO_TEST_CASE( test_search_boyer_moore )
    {
        const auto search_keys = std::vector<std::string>{"name", "tag"};
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("search")},
                         {std::string("search_term"), std::string("output")},
                         {std::string("search_keys"), search_keys},
                         {std::string("search_algorithm"), std::string("partial")},
                         {std::string("max_results"), 20000},
                 }}
        );
        auto response = TagHierarchy::Handle(query);
        BOOST_TEST(response.size() == 36);
    }
    BOOST_AUTO_TEST_CASE( test_search_exact )
    {
        const auto search_keys = std::vector<std::string>{"parent_id"};
        // Level1-1
        const auto search_term = std::string("eba77720-4ba5-4803-9834-ef0faf40f057");
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("search")},
                         {std::string("search_term"), search_term},
                         {std::string("search_keys"), search_keys},
                         {std::string("search_algorithm"), std::string("exact")},
                         {std::string("max_results"), 1},
                 }}
        );
        auto response = TagHierarchy::Handle(query);
        const auto result = response[0];
        BOOST_TEST(boost::get<std::string>(result.at("parent_id")) == search_term);
        BOOST_TEST(response.size() == 1);
    }

BOOST_AUTO_TEST_SUITE_END()
