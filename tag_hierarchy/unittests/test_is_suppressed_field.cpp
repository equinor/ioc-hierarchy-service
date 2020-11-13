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
void verify_correct_suppression_relationship(bool isParentSuppressed,
                                             const std::string& parentId,
                                             const std::vector<NodeType>& command) {
    // We verify this relationship by obtaining the suppression state of all children,
    // and checking if at least one of these are true. If that is the case, the parent
    // should be suppressed too. If not, the parent should not be suppressed.

    // Make a query containing the parent ID, and insert the command.
    auto childrenQuery = std::vector<NodeType>(
            {{
                {std::string("parentId"), parentId}
            }}
            );

    childrenQuery[0].insert(command[0].begin(), command[0].end());

    auto childrenQueryResponse = TagHierarchy::Handle(childrenQuery);

    // Did the query return any child nodes?
    if (childrenQueryResponse.empty()) {
        // No, then we return. This is the stop condition of the recursion.
        return;
    }

    // Aggregate the suppression state of the children. This determines the value that
    // the parent _should_ have.
    bool shouldParentBeSuppressed = false;
    for (auto const& node: childrenQueryResponse) {
        // Aggregate the suppression value from the children of the parent node.
        bool isChildSuppressed = boost::get<bool>(node.find("issuppressed")->second);
        shouldParentBeSuppressed |= isChildSuppressed;

        // Verify the same for all the children.
        std::string nodeId = boost::get<std::string>(node.find("id")->second);
        verify_correct_suppression_relationship(
                isChildSuppressed,
                nodeId,
                command
                );
    }

    // Verify that the suppression is as it should be in the parent.
    BOOST_TEST(isParentSuppressed == shouldParentBeSuppressed);
}

// Verify that the suppression relationship is correct for input filter.
void verify_correct_suppression_relationship_with_filter(const std::vector<NodeType>& filter) {
    // Construct the query for the top nodes.
    auto query = std::vector<NodeType>(
            {{
                {std::string("command"), std::string("nodes")},
                {std::string("parentId"), pybind11::none()}
             }}
    );
    for (const auto& filterEntry : filter) {
        query[0].insert(filterEntry.begin(), filterEntry.end());
    }

    auto response = TagHierarchy::Handle(query);

    // Construct the command to pass on when querying for child nodes.
    auto childNodesCommand = std::vector<NodeType>(
            {{
                     {std::string("command"), std::string("nodes")}
             }}
    );

    // Insert filter into the command.
    for (const auto& filterEntry : filter) {
        childNodesCommand[0].insert(filterEntry.begin(), filterEntry.end());
    }

    // Verify the suppression relationship for all nodes in the query result.
    for (const auto& props : response) {
        std::string nodeId = boost::get<std::string>(props.find("id")->second);
        verify_correct_suppression_relationship(
                boost::get<bool>(props.find("issuppressed")->second),
                nodeId,
                childNodesCommand
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
                        std::string("modelownerfilter"), std::vector<std::string>{"8eb4a1e1-1316-bcef-b20f-bbbaed2a4e95"}
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
                             std::string("modelclassfilter"), std::vector<std::string>{"44380be8-76d2-a1fd-dc50-0ad8f4c151e4"}
                         }
                }}
                ));
    }

BOOST_AUTO_TEST_SUITE_END()