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
                                         std::string("modelownerfilter"), std::vector<std::string>{"8eb4a1e1-1316-bcef-b20f-bbbaed2a4e95"}
                                 }
                         }}
                ));
        }

// Tests that the field for severity is propagated from model element nodes
// all the way up to the root node, when the query contains a model class filter.
BOOST_AUTO_TEST_CASE( test_severity_propagation_with_model_class_filter )
        {
                verify_correct_severity_level_with_filter(std::vector<NodeType>(
                        {{
                                 {
                                         std::string("modelclassfilter"), std::vector<std::string>{"44380be8-76d2-a1fd-dc50-0ad8f4c151e4"}
                                 }
                         }}
                ));
        }

BOOST_AUTO_TEST_SUITE_END()


