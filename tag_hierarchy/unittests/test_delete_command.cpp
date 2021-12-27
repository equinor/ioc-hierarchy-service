#include "models/models.h"

#include <boost/test/unit_test.hpp>

#include "tag_hierarchy/unittests/testutils.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"


BOOST_FIXTURE_TEST_SUITE( DeleteCommandTest, Fixture );
    BOOST_AUTO_TEST_CASE( test_delete_top_level_node )
    {
        // Make a command that deletes the node named Level1-2
        const auto node_id = std::string("2a346481-f5a7-48c4-8ccd-c3685a68189e");
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({node_id})}}}
        );
        auto response = TagHierarchy::Handle(query);
        // Ensure status is success
        BOOST_TEST(boost::get<std::string>(response[0].at(node_id)) == "Success");

        // Get all the top level nodes
        auto query2 = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), pybind11::none()}}}
        );
        auto response2 = TagHierarchy::Handle(query2);
        // And ensure that Level1-1 is still there, and there are no more, since Level1-2 should now be deleted
        BOOST_TEST(boost::get<std::string>(response2[0].at("name")) == "Level1-1");
        BOOST_TEST(response2.size() == 1);
    }

    BOOST_AUTO_TEST_CASE( test_delete_model_and_verify_it_is_missing )
    {
        // Make a command that deletes the node named Level1-1->Level2-1->Level3-1 (it has the guid below)
        const auto node_id = std::string("23dce722-9aac-489a-9222-aae2c51f91c8");
        const auto parent_node_id = std::string("55b7d593-d163-4649-84ee-afb193045aa0");
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                      {std::string("nodes"), std::vector<std::string>({node_id})}}}
        );
        auto response = TagHierarchy::Handle(query);

        // Verify that job is reported successful
        BOOST_TEST(boost::get<std::string>(response[0].at(node_id)) == "Success");

        // Get the children of the parent of that model
        auto query2 = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                  {std::string("parentId"), std::string(parent_node_id)}}}
        );
        response = TagHierarchy::Handle(query2);

        // And verify that our deleted node is not among the children anymore
        auto predicate = [](std::vector<NodeType> response) -> bool {
            auto found_node = std::find_if(response.begin(), response.end(), [](const NodeType& node){
                return boost::get<std::string>(node.at("name")) == "Level1-2->Level2-3->Level3-1";
            });
            return found_node == response.end();
        };
        BOOST_CHECK_PREDICATE(predicate, (response));
    }

    BOOST_AUTO_TEST_CASE( test_delete_model_twice_is_ok ) {
        // Delete a node in the tree (Level1-2->Level2-3)
        const auto node_id = std::string("14ab5d20-db2e-4391-ab46-250e194b3eb0");
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({node_id})}}}
        );
        // Call the command twice
        auto response = TagHierarchy::Handle(query);
        response = TagHierarchy::Handle(query);
        // We should not crash, we should just get a response that the node is now "Not found" since
        // it has already been deleted
        BOOST_TEST(boost::get<std::string>(response[0].at(node_id)) == "Not found");
    }

BOOST_AUTO_TEST_SUITE_END()
