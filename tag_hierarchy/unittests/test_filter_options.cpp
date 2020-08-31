#include <boost/test/unit_test.hpp>

#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"


BOOST_FIXTURE_TEST_SUITE( FilterOptionsTest, Fixture );
BOOST_AUTO_TEST_CASE( test_modelowner_filter )
{
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("filteroptions")},
          {std::string("type"), std::string("modelowner")}}}
    );
    auto response = TagHierarchy::Handle(query);
    const auto response_map = boost::get<NodeType>(response[0]);
    const auto compare = boost::get<std::vector<std::string>>(response_map.at("modelowner_ids"));
    const auto baseline = std::vector<std::string>{"8eb4a1e1-1316-bcef-b20f-bbbaed2a4e95",
                                                   "ff1b9df9-60ae-7825-89e6-36998289d1fa"};
    BOOST_TEST(baseline == compare, boost::test_tools::per_element());

}

BOOST_AUTO_TEST_CASE( test_modelclass_filter )
{
  auto query = std::vector<NodeType>(
                                     {{{std::string("command"), std::string("filteroptions")},
                                       {std::string("type"), std::string("modelclass")}}}
                                     );
  auto response = TagHierarchy::Handle(query);
  const auto response_map = boost::get<NodeType>(response[0]);
  const auto compare = boost::get<std::vector<std::string>>(response_map.at("modelclass_ids"));
  const auto baseline = std::vector<std::string>{"44380be8-76d2-a1fd-dc50-0ad8f4c151e4",
                                                 "ff1cc41e-a134-2426-600d-7199bcb509c5"};
  BOOST_TEST(baseline == compare, boost::test_tools::per_element());

}

BOOST_AUTO_TEST_SUITE_END()
