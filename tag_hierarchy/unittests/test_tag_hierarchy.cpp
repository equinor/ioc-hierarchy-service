#define BOOST_TEST_MODULE tag_hierarchy
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"

struct Fixture {
  Fixture() {
     BOOST_TEST_MESSAGE( "Constructing test fixture" );
     auto modelhierarchy = std::vector<NodeType>(
     #include "hierarchy_dump.cpp"
     );
     tag_hierarchy.Handle(modelhierarchy);
  }
  //void setup() {
  //   BOOST_TEST_MESSAGE( "Populating graph" );
  //}
  //void teardown() {
  //   BOOST_TEST_MESSAGE( "Tearing down test suite" );
  //}

  ~Fixture() {
      BOOST_TEST_MESSAGE( "teardown fixture" );
  }

  static TagHierarchy tag_hierarchy;
  static std::string next_parent_id;
};

TagHierarchy Fixture::tag_hierarchy = TagHierarchy();
std::string Fixture::next_parent_id = std::string();

BOOST_GLOBAL_FIXTURE( Fixture );
BOOST_AUTO_TEST_CASE( test_nodes )
{
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), pybind11::none()}}}
    );
    auto response = Fixture::tag_hierarchy.Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-1");
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 1);
    BOOST_TEST(boost::get<pybind11::none>(response[1].at("parent_id")) == pybind11::none());
    Fixture::next_parent_id = boost::get<std::string>(response[1].at("id"));
    // BOOST_TEST(boost::get<std::string>(response[1].at("is_modelelement")) == false);
}

BOOST_AUTO_TEST_CASE( test_l2_nodes ) {
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), Fixture::next_parent_id}}}
    );
    auto response = Fixture::tag_hierarchy.Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-2->Level2-1");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 2);
    BOOST_TEST(boost::get<std::string>(response[1].at("parent_id")) == Fixture::next_parent_id);
    Fixture::next_parent_id = boost::get<std::string>(response[0].at("id"));
}

BOOST_AUTO_TEST_CASE( test_l3_nodes ) {
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), Fixture::next_parent_id}}}
    );
    auto response = Fixture::tag_hierarchy.Handle(query);
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2->Level2-1->Level3-2");
    BOOST_TEST(boost::get<int>(response[0].at("levelno")) == 3);
    BOOST_TEST(boost::get<std::string>(response[1].at("parent_id")) == Fixture::next_parent_id);
    //BOOST_TEST(boost::get<bool>(response[0].at("is_modelelement")) == false;
    Fixture::next_parent_id = boost::get<std::string>(response[1].at("id"));
}

BOOST_AUTO_TEST_CASE( test_l4_nodes ) {
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), Fixture::next_parent_id}}}
    );
    // TODO make support for model element folders
    // assert response_l4.data[0]['levelno'] == 4
    // assert response_l4.data[0]['type'] == NodeType.MODELELEMENTFOLDER
    // assert response_l4.data[1]['is_modelelement'] is True
    // assert response_l4.data[0]['parent_id'] == response_l3.data[1]['id']
    // assert response_l4.data[1]['parent_id'] == response_l4.data[0]['id']
}
