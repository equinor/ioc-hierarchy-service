#include "tag_hierarchy/unittests/fixture.h"

#include <boost/test/unit_test.hpp>

#include "models/models.h"
#include "tag_hierarchy/tag_hierarchy.h"

Fixture::Fixture() {
    BOOST_TEST_MESSAGE( "Constructing test fixture" );
    auto modelhierarchy = std::vector<NodeType>(
#include "hierarchy_dump.cpp"
        );
    TagHierarchy::Handle(modelhierarchy);
}

Fixture::~Fixture() {
    BOOST_TEST_MESSAGE( "teardown fixture" );
    auto flush_command = std::vector<NodeType>(
                                               {{{std::string("command"), std::string("flush")}}}
                                               );
    TagHierarchy::Handle(flush_command);
}
