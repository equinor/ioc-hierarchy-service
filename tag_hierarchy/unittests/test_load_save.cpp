#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE( LoadSaveCommandsTest, Fixture );
    BOOST_AUTO_TEST_CASE( test_load_save_command )
    {
        // Store and restore the graph using the commands
        auto store_command = std::vector<NodeType>(
                {{{std::string("command"), std::string("store")}}}
        );
        auto store_response = TagHierarchy::Handle(store_command);
        const auto& serialized_graph = boost::get<std::string>(store_response[0]["serialized_graph"]);
        auto restore_command = std::vector<NodeType>(
                {{{std::string("command"), std::string("restore")},
                         {std::string("serialized_hierarchy"), serialized_graph}}}
        );
        auto restore_response = TagHierarchy::Handle(restore_command);

        auto query2 = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), pybind11::none()}}}
        );
        auto response2 = TagHierarchy::Handle(query2);
        BOOST_TEST(boost::get<std::string>(response2[0].at("name")) == "Level1-1");
        BOOST_TEST(response2.size() == 2);

        // Verify that vertices are still correctly looked up by ID (Level1-2->Level2-1)
        const auto parent_id = std::string("dc9a72d9-50c6-463c-823b-d707abb321da");
        auto l3query = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), parent_id}}}
        );
        auto l3response = TagHierarchy::Handle(l3query);
        BOOST_TEST(boost::get<std::string>(l3response[0].at("name")) == "Level1-2->Level2-1->Level3-1");
        BOOST_TEST(boost::get<int>(l3response[0].at("levelno")) == 3);
        BOOST_TEST(boost::get<std::string>(l3response[0].at("parent_id")) == parent_id);
        std::cout << boost::get<std::string>(l3response[0].at("id"));
    }

BOOST_AUTO_TEST_SUITE_END()
