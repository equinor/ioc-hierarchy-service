//
// Created by Erik Kvam Måland on 02/06/2021.
//

#include <boost/test/unit_test.hpp>

#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

// Used for verifying that the values of field for severity level of nodes
// is propagated to through the hierarchy as expected.
void verify_correct_node_severity_level(int parent_severity_level,
                                        const std::string& parent_id,
                                        const std::vector<NodeType>& command) {
    // We verify the severity level by obtaining the severity levels of all children,
    // and checking that the severity level of the parent is the maximum of these.

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

    int expected_parent_severity_level = -1;
    for (auto const& node: children_query_response) {
        int child_severity_level = -1;
        if (node.count("severity") > 0) {
            child_severity_level = boost::get<int>(node.find("severity")->second);
        }
        expected_parent_severity_level = std::max(expected_parent_severity_level, child_severity_level);

        // Verify the same for all the children.
        std::string node_id = boost::get<std::string>(node.find("id")->second);
        verify_correct_node_severity_level(
                child_severity_level,
                node_id,
                command
        );
    }

    BOOST_TEST(expected_parent_severity_level == parent_severity_level);
}

void verify_correct_severity_level_with_filter(const std::vector<NodeType>& filter) {

    // Update the tag hierarchy with nodes that have severity level set.
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
                        {std::string("severity"), 1 }
                    },
                    {
                        {std::string("name"),std::string("Level1-2->Level2-3->Level3-2->L4Model-1-input1-element-2")},
                        {std::string("description"),std::string("Description")},
                        {std::string("id"),std::string("a8127a40-2a28-2ed1-83e9-442ada88c496")},
                        {std::string("parent_id"),std::string("b26b3f36-ade0-a404-681f-1d50020d64eb")},
                        {std::string("levelno"), 4 },
                        {std::string("is_modelelement"), true },
                        {std::string("elementtype"),std::string("Database")},
                        {std::string("type"),std::string("modelElement")},
                        {std::string("isrunningtag"), false },
                        {std::string("kpigroup"),std::string("Temporib")},
                        {std::string("kpigroup_id"),std::string("d9174b0d-2519-423f-c933-7491cce63858")},
                        {std::string("issuppressed"), true },
                        {std::string("severity"), 0 }
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

    for (const auto& props : response) {
        std::string node_id = boost::get<std::string>(props.find("id")->second);
        int severity_level = -1;
        if (props.count("severity") > 0) {
            severity_level = boost::get<int>(props.find("severity")->second);
        }
        verify_correct_node_severity_level(
                severity_level,
                node_id,
                child_nodes_command
        );
    }
}

BOOST_FIXTURE_TEST_SUITE( FieldSeverityTest, Fixture );

    // Tests that the field for severity is propagated from
    // model element nodes all the way up to the root node.
    BOOST_AUTO_TEST_CASE( test_severity_propagation )
    {
        verify_correct_severity_level_with_filter(std::vector<NodeType>());
    }

    // Tests that the field for severity is propagated from model element nodes
    // all the way up to the root node, when the query contains a model owner filter.
    BOOST_AUTO_TEST_CASE( test_severity_propagation_with_model_owner_filter )
    {
        verify_correct_severity_level_with_filter(std::vector<NodeType>(
                {{
                    {
                        std::string("modelownerfilter"), std::vector<std::string>{"3627b2f8-050b-43ec-8d3f-cadfb652880b"}
                    }
                }}
                ));
    }

    // Tests that the field for severity is propagated from model element nodes
    // all the way up to the root node, when the query contains a model class filter.
    BOOST_AUTO_TEST_CASE( test_severity_propagation_with_model_class_filter )
    {
        verify_correct_severity_level_with_filter(std::vector<NodeType>({{
                    {
                    std::string("modelclassfilter"), std::vector<std::string>{"56ce0ce9-596e-43d2-8e6c-4ece3e81b20a"}
                    }
            }}
            ));
    }

BOOST_AUTO_TEST_SUITE_END()


