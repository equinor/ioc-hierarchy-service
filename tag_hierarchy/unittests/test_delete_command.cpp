#include "models/models.h"

#include <boost/test/unit_test.hpp>

#include "tag_hierarchy/unittests/testutils.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"


BOOST_FIXTURE_TEST_SUITE( DeleteCommandTest, Fixture );
    BOOST_AUTO_TEST_CASE( test_delete_top_level_node )
    {
        // Make a command that deletes the node named Level1-2
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({"33382bc4-249b-a646-ef2b-14033605bae0"})}}}
        );
        auto response = TagHierarchy::Handle(query);
        // Ensure status is success
        BOOST_TEST(boost::get<std::string>(response[0].at("33382bc4-249b-a646-ef2b-14033605bae0")) == "Success");

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
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({"cae8efee-f222-78c1-7480-86cd49b7dc3f"})}}}
        );
        auto response = TagHierarchy::Handle(query);

        // Verify that job is reported successful
        BOOST_TEST(boost::get<std::string>(response[0].at("cae8efee-f222-78c1-7480-86cd49b7dc3f")) == "Success");

        // Get the children of the parent of that model
        auto query2 = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), std::string("be470052-4352-c1f9-e6b2-adef4dbd966e")}}}
        );
        response = TagHierarchy::Handle(query2);

        // And verify that our deleted node is not among the children anymore
        auto predicate = [](std::vector<NodeType> response) -> bool {
            auto found_node = std::find_if(response.begin(), response.end(), [](const NodeType& node){
                return boost::get<std::string>(node.at("name")) == "Level1-1->Level2-1->Level3-1";
            });
            return found_node == response.end();
        };
        BOOST_CHECK_PREDICATE(predicate, (response));
    }

    BOOST_AUTO_TEST_CASE( test_delete_model_twice_is_ok ) {
        // Delete a node in the tree
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({"cae8efee-f222-78c1-7480-86cd49b7dc3f"})}}}
        );
        // Call the command twice
        auto response = TagHierarchy::Handle(query);
        response = TagHierarchy::Handle(query);
        // We should not crash, we should just get a response that the node is now "Not found" since
        // it has already been deleted
        BOOST_TEST(boost::get<std::string>(response[0].at("cae8efee-f222-78c1-7480-86cd49b7dc3f")) == "Not found");
    }

BOOST_AUTO_TEST_SUITE_END()
