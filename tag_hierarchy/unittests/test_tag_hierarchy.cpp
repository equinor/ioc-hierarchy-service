#define BOOST_TEST_MODULE tag_hierarchy_test
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>


#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"


BOOST_FIXTURE_TEST_SUITE( NodesTest, Fixture );
BOOST_AUTO_TEST_CASE( test_nodes )
{
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), pybind11::none()}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-1");
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 1);
    BOOST_TEST(boost::get<pybind11::none>(response[1].at("parent_id")) == pybind11::none());
}

BOOST_AUTO_TEST_CASE( test_l2_nodes ) {
    const auto parent_id = std::string("2a346481-f5a7-48c4-8ccd-c3685a68189e");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-2->Level2-1");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 2);
    BOOST_TEST(boost::get<std::string>(response[1].at("parent_id")) == parent_id);
}

BOOST_AUTO_TEST_CASE( test_l3_nodes ) {
    const auto parent_id = std::string("dc9a72d9-50c6-463c-823b-d707abb321da");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-2->Level2-1->Level3-1");
    BOOST_TEST(boost::get<int>(response[0].at("levelno")) == 3);
    BOOST_TEST(boost::get<std::string>(response[0].at("parent_id")) == parent_id);
    std::cout << boost::get<std::string>(response[0].at("id"));
}

BOOST_AUTO_TEST_CASE( test_l4_nodes ) {
    const auto parent_id = std::string("446ae21e-f27a-4578-8022-3e9f5a01108c");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<int>(response[0].at("levelno")) == 4);
}

BOOST_AUTO_TEST_SUITE_END()
