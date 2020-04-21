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
    const auto parent_id = std::string("33382bc4-249b-a646-ef2b-14033605bae0");
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
    const auto parent_id = std::string("0979534e-9dc1-bf0e-208a-d508862888fe");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2->Level2-1->Level3-2");
    BOOST_TEST(boost::get<int>(response[0].at("levelno")) == 3);
    BOOST_TEST(boost::get<std::string>(response[1].at("parent_id")) == parent_id);
    std::cout << boost::get<std::string>(response[1].at("id"));
}

BOOST_AUTO_TEST_CASE( test_l4_nodes ) {
    const auto parent_id = std::string("0b8b675f-0afb-7772-6409-81a069aedb2a");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<int>(response[0].at("levelno")) == 4);
}

BOOST_AUTO_TEST_SUITE_END()
