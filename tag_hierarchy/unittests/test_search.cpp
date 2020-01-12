#define BOOST_TEST_MODULE tag_hierarchy
#define BOOST_TEST_DYN_LINK

#include "models/models.h"

#include <boost/test/unit_test.hpp>

#include "tag_hierarchy/tag_hierarchy.h"

struct SearchFixture {
  SearchFixture() {
     BOOST_TEST_MESSAGE( "Constructing test fixture" );
     auto modelhierarchy = std::vector<NodeType>(
     #include "hierarchy_dump.cpp"
     );
     TagHierarchy::Handle(modelhierarchy);
  }

  ~SearchFixture() {
      BOOST_TEST_MESSAGE( "teardown fixture" );
  }
};

BOOST_FIXTURE_TEST_SUITE( SearchTest, SearchFixture );
BOOST_AUTO_TEST_CASE( test_search )
{
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("search")},
          {std::string("search_term"), std::string("Level1")}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-1");
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 1);
    BOOST_TEST(boost::get<pybind11::none>(response[1].at("parent_id")) == pybind11::none());
    //Fixture::next_parent_id = boost::get<std::string>(response[1].at("id"));
    // BOOST_TEST(boost::get<std::string>(response[1].at("is_modelelement")) == false);
}

BOOST_AUTO_TEST_SUITE_END()
