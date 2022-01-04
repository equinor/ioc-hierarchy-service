#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE( UpdateTest, Fixture );
    BOOST_AUTO_TEST_CASE( test_update_node )
    {
        const auto node_id = std::string("eba77720-4ba5-4803-9834-ef0faf40f057");
        auto query = std::vector<NodeType>(
                {
                    {{std::string("command"), std::string("update")},
                 },
                    {{std::string("name"),std::string("Level1-1-newname")},
                    {std::string("id"),std::string(node_id)},
                    {std::string("parent_id"), pybind11::none()},
                    {std::string("levelno"), 1 },
                    {std::string("is_modelelement"), false },
                    {std::string("type"),std::string("folder")},
                    {std::string("modelowner"), pybind11::none()},
                    {std::string("modelclass"), pybind11::none()},
                    {std::string("modelsource_id"), pybind11::none()} },
                 }
        );
        auto response = TagHierarchy::Handle(query);
        // Verify that we have succeeded
        BOOST_TEST(boost::get<std::string>(response[0].at("ok")) == std::string("true"));
        // Verify that node was modified
        query = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), pybind11::none()}}}
        );
        response = TagHierarchy::Handle(query);

        // Find the node which was attempted to be modified
        auto found_node = bool{false};
        for (auto node: response) {
            if(node.at("id") == Variants(std::string(node_id))) {
                // Assert that it has got the new name
                BOOST_ASSERT(node["name"] == Variants{std::string("Level1-1-newname")});
                found_node = true;
            }
        }
        // Assert that the node was actually found in the response
        BOOST_ASSERT(found_node);
    };

    BOOST_AUTO_TEST_CASE( test_update_node_cannot_change_parent_id )
    {
        const auto node_id = std::string("eba77720-4ba5-4803-9834-ef0faf40f057");
        const auto search_keys = std::vector<std::string>{"name", "tag"};
        auto query = std::vector<NodeType>(
                {
                        {{std::string("command"), std::string("update")},
                        },
                        {{std::string("id"),std::string(node_id)},
                                {std::string("parent_id"), std::string(node_id)},
                                {std::string("levelno"), 1 },
                                {std::string("is_modelelement"), false },
                                {std::string("type"),std::string("folder")},
                                {std::string("modelowner"), pybind11::none()},
                                {std::string("modelclass"), pybind11::none()},
                                {std::string("modelsource_id"), pybind11::none()} },
                }
        );
        auto response = TagHierarchy::Handle(query);
        // Check that error was thrown
        BOOST_ASSERT(response[0].count("error") > 0);
        // Verify that node was not changed
        query = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), pybind11::none()}}}
        );
        response = TagHierarchy::Handle(query);

        // Find the node which was attempted to be modified
        auto found_node = bool{false};
        for (auto node: response) {
            if(node.at("id") == Variants(std::string(node_id))) {
                // Assert that it still has parent id none, as before modification
                BOOST_ASSERT(node["parent_id"] == Variants{pybind11::none()});
                found_node = true;
            }
        }
        // Assert that the node was actually found in the response
        BOOST_ASSERT(found_node);
    };

BOOST_AUTO_TEST_SUITE_END()
