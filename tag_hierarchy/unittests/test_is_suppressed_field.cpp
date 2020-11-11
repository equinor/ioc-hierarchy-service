//
// Created by Erik Måland (erma@equinor.com) on 09.11.2020.
//
// Tests for verifying that the field for alarm suppression works as expected.

#include <boost/test/unit_test.hpp>

#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

// Used for verifying that the values of field for alarm suppression
// is valid for a node with input ID.
static void verify_correct_suppression_relationship(std::string parentId, bool isParentSuppressed) {
    // We verify this relationship by obtaining the suppression state of all children,
    // and checking if at least one of these are true. If that is the case, the parent
    // should be suppressed too. If not, the parent should not be suppressed.

    // Obtain the children
    auto childrenQuery = std::vector<NodeType>(
            {{{std::string("command"), std::string("nodes")},
                     {std::string("parentId"), std::string(parentId)}}});

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
        verify_correct_suppression_relationship(
                boost::get<std::string>(node.find("id")->second),
                isChildSuppressed);
    }

    // Verify that the suppression is as it should be in the parent.
    BOOST_TEST(isParentSuppressed == shouldParentBeSuppressed);
}

BOOST_FIXTURE_TEST_SUITE( FieldIsSuppressedTest, Fixture );

    // Tests that the field for alarm suppression is propagated from
    // model element nodes all the way up to the root node.
    BOOST_AUTO_TEST_CASE( test_is_suppressed_propagation )
    {
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), pybind11::none()}}}
        );
        auto response = TagHierarchy::Handle(query);

        for (const auto& props : response) {
            verify_correct_suppression_relationship(
                    boost::get<std::string>(props.find("id")->second),
                    boost::get<bool>(props.find("issuppressed")->second)
                            );
        }
    }

    // Tests that the field for alarm suppression is propagated from
    // model element nodes all the way up to the root node, when the query
    // contains an l1 filter.
    BOOST_AUTO_TEST_CASE( test_is_suppressed_propagation_with_l1_filter )
    {
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), pybind11::none()}}}
        );
        auto response = TagHierarchy::Handle(query);

        for (const auto& props : response) {
            verify_correct_suppression_relationship(
                    boost::get<std::string>(props.find("id")->second),
                    boost::get<bool>(props.find("issuppressed")->second)
            );
        }
    }

BOOST_AUTO_TEST_SUITE_END()