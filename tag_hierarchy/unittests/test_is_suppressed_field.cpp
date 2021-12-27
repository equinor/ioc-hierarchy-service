//
// Created by Erik Måland (erma@equinor.com) on 09.11.2020.
//
// Tests for verifying that the field for alarm suppression works as expected.

#include <boost/test/unit_test.hpp>

#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

// Used for verifying that the values of field for alarm suppression
// is valid for a nodes obtained with input command to cache.
void verify_correct_suppression_relationship(bool is_parent_suppressed,
                                             const std::string& parent_id,
                                             const std::vector<NodeType>& command) {
    // We verify this relationship by obtaining the suppression state of all children,
    // and checking if at least one of these are true. If that is the case, the parent
    // should be suppressed too. If not, the parent should not be suppressed.

    // Make a query containing the parent ID, and insert the command.
    auto children_query = std::vector<NodeType>(
            {{
                {std::string("parentId"), parent_id}
            }}
            );

    children_query[0].insert(command[0].begin(), command[0].end());

    auto children_query_response = TagHierarchy::Handle(children_query);

    // Did the query return any child nodes?
    if (children_query_response.empty()) {
        // No, then we return. This is the stop condition of the recursion.
        return;
    }

    // Aggregate the suppression state of the children. This determines the value that
    // the parent _should_ have.
    bool should_parent_be_suppressed = false;
    for (auto const& node: children_query_response) {
        // Aggregate the suppression value from the children of the parent node.
        bool is_child_suppressed = boost::get<bool>(node.find("issuppressed")->second);
        should_parent_be_suppressed |= is_child_suppressed;

        // Verify the same for all the children.
        std::string node_id = boost::get<std::string>(node.find("id")->second);
        verify_correct_suppression_relationship(
                is_child_suppressed,
                node_id,
                command
                );
    }

    // Verify that the suppression is as it should be in the parent.
    BOOST_TEST(is_parent_suppressed == should_parent_be_suppressed);
}

// Verify that the suppression relationship is correct for input filter.
void verify_correct_suppression_relationship_with_filter(const std::vector<NodeType>& filter) {

    // Update the tag hierarchy with nodes to have issuppressed set.
    auto updateQuery = std::vector<NodeType>(
            {
                    {{std::string("command"), std::string("update")},
                    },
                    {
                        {std::string("name"),std::string("Level1-2->Level2-3->Level3-1->L4Model-1-input0-element-1")},
                        {std::string("description"),std::string("Description")},
                        {std::string("id"),std::string("37eb955a-5866-4757-b5b1-497497c74325")},
                        {std::string("parent_id"),std::string("a725ab7c-c4c3-47e4-81af-526bf50b8e05")},
                        {std::string("levelno"), 4 },
                        {std::string("is_modelelement"), true },
                        {std::string("elementtype"),std::string("Database")},
                        {std::string("type"),std::string("modelElement")},
                        {std::string("isrunningtag"), false },
                        {std::string("kpigroup"),std::string("Temporib")},
                        {std::string("kpigroup_id"),std::string("f9479292-b852-401f-9e34-33cc2a7decb6")},
                        {std::string("issuppressed"), false },
                        {std::string("severity"), 1 }
                    },
                    {
                        {std::string("name"),std::string("Level1-2->Level2-3->Level3-2->L4Model-1-input1-element-1")},
                        {std::string("description"),std::string("Description")},
                        {std::string("id"),std::string("51f71096-62b5-4e08-a250-d7214feef245")},
                        {std::string("parent_id"),std::string("4a265338-a12b-4736-97ad-88e4cc3308ed")},
                        {std::string("levelno"), 4 },
                        {std::string("is_modelelement"), true },
                        {std::string("elementtype"),std::string("Database")},
                        {std::string("type"),std::string("modelElement")},
                        {std::string("isrunningtag"), false },
                        {std::string("kpigroup"),std::string("Temporib")},
                        {std::string("kpigroup_id"),std::string("f9479292-b852-401f-9e34-33cc2a7decb6")},
                        {std::string("issuppressed"), true },
                        {std::string("severity"), 2 }
                    },
                    {
                        {std::string("name"),std::string("Level1-2->Level2-3->Level3-3->L4Model-1-output0-element-1")},
                        {std::string("description"),std::string("Description")},
                        {std::string("id"),std::string("8639784c-7bd3-488f-9a17-5c6f0dbaec32")},
                        {std::string("parent_id"),std::string("1ee52fee-07bc-4bdb-88ac-711708c2f5fc")},
                        {std::string("levelno"), 4 },
                        {std::string("is_modelelement"), true },
                        {std::string("elementtype"),std::string("Calculated")},
                        {std::string("type"),std::string("modelElement")},
                        {std::string("isrunningtag"), false },
                        {std::string("kpigroup"),std::string("Temporib")},
                        {std::string("kpigroup_id"),std::string("f9479292-b852-401f-9e34-33cc2a7decb6")},
                        {std::string("issuppressed"), true },
                        {std::string("severity"), 3 }
                    },
            }
    );

    TagHierarchy::Handle(updateQuery);

    // Construct the query for the top nodes.
    auto query = std::vector<NodeType>(
            {{
                {std::string("command"), std::string("nodes")},
                {std::string("parentId"), pybind11::none()}
             }}
    );
    for (const auto& filter_entry : filter) {
        query[0].insert(filter_entry.begin(), filter_entry.end());
    }

    auto response = TagHierarchy::Handle(query);

    // Construct the command to pass on when querying for child nodes.
    auto child_nodes_command = std::vector<NodeType>(
            {{
                     {std::string("command"), std::string("nodes")}
             }}
    );

    // Insert filter into the command.
    for (const auto& filter_entry : filter) {
        child_nodes_command[0].insert(filter_entry.begin(), filter_entry.end());
    }

    // Verify the suppression relationship for all nodes in the query result.
    for (const auto& props : response) {
        std::string node_id = boost::get<std::string>(props.find("id")->second);
        verify_correct_suppression_relationship(
                boost::get<bool>(props.find("issuppressed")->second),
                node_id,
                child_nodes_command
        );
    }
}

BOOST_FIXTURE_TEST_SUITE( FieldIsSuppressedTest, Fixture );

    // Tests that the field for alarm suppression is propagated from
    // model element nodes all the way up to the root node.
    BOOST_AUTO_TEST_CASE( test_is_suppressed_propagation )
    {
        verify_correct_suppression_relationship_with_filter(std::vector<NodeType>());
    }

    // Tests that the field for alarm suppression is propagated from
    // model element nodes all the way up to the root node, when the query
    // contains a model owner filter.
    BOOST_AUTO_TEST_CASE( test_is_suppressed_propagation_with_model_owner_filter )
    {
        verify_correct_suppression_relationship_with_filter(std::vector<NodeType>(
                {{
                    {
                        std::string("modelownerfilter"), std::vector<std::string>{"3627b2f8-050b-43ec-8d3f-cadfb652880b"}
                    }
                }}
                ));
    }

    // Tests that the field for alarm suppression is propagated from
    // model element nodes all the way up to the root node, when the query
    // contains a model class filter.
    BOOST_AUTO_TEST_CASE( test_is_suppressed_propagation_with_model_class_filter )
    {
        verify_correct_suppression_relationship_with_filter(std::vector<NodeType>(
                {{
                         {
                             std::string("modelclassfilter"), std::vector<std::string>{"56ce0ce9-596e-43d2-8e6c-4ece3e81b20a"}
                         }
                }}
                ));
    }

BOOST_AUTO_TEST_SUITE_END()
