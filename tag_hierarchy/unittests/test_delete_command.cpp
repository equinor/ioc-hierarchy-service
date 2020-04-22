#include "models/models.h"

#include <boost/test/unit_test.hpp>

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

BOOST_FIXTURE_TEST_SUITE( DeleteCommandTest, Fixture );
    BOOST_AUTO_TEST_CASE( test_delete_command )
    {
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({"33382bc4-249b-a646-ef2b-14033605bae0"})}}}
        );
        auto response = TagHierarchy::Handle(query);
        BOOST_TEST(boost::get<std::string>(response[0].at("33382bc4-249b-a646-ef2b-14033605bae0")) == "Success");

        auto query2 = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), pybind11::none()}}}
        );
        auto response2 = TagHierarchy::Handle(query2);
        BOOST_TEST(boost::get<std::string>(response2[0].at("name")) == "Level1-1");
        BOOST_TEST(response2.size() == 1);
    }

BOOST_AUTO_TEST_SUITE_END()
