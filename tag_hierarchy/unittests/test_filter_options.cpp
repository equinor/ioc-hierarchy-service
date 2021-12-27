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
    const auto baseline = std::vector<std::string>{"3627b2f8-050b-43ec-8d3f-cadfb652880b",
                                                   "f0424bb3-ed3a-4f3a-ae60-1250d4b74a60"};
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
  const auto baseline = std::vector<std::string>{"13c3727d-b8af-4dc9-95a1-c9cc3de5b9af",
                                                 "56ce0ce9-596e-43d2-8e6c-4ece3e81b20a"};
  BOOST_TEST(baseline == compare, boost::test_tools::per_element());

}

BOOST_AUTO_TEST_SUITE_END()
