#include "tag_hierarchy/unittests/fixture.h"

#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/variant.hpp>

#include "models/models.h"
#include "tag_hierarchy/tag_hierarchy.h"

Fixture::Fixture() {
    BOOST_TEST_MESSAGE( "Constructing test fixture" );
    auto modelhierarchy = std::vector<NodeType>();
    std::ifstream input("unittests/serialized_hierarchy.dat");
    boost::archive::text_iarchive ia(input);
    ia >> modelhierarchy;
    TagHierarchy::Handle(modelhierarchy);
}

Fixture::~Fixture() {
    BOOST_TEST_MESSAGE( "teardown fixture" );
    auto flush_command = std::vector<NodeType>(
                                               {{{std::string("command"), std::string("flush")}}}
                                               );
    TagHierarchy::Handle(flush_command);
}
