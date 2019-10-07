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
     TagHierarchy::Handle(modelhierarchy);
  }

  ~Fixture() {
      BOOST_TEST_MESSAGE( "teardown fixture" );
  }
};

BOOST_FIXTURE_TEST_SUITE( NodesTest, Fixture );
BOOST_AUTO_TEST_CASE( test_nodes )
{
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), pybind11::none()}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-1");
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 1);
    BOOST_TEST(boost::get<pybind11::none>(response[1].at("parent_id")) == pybind11::none());
    //Fixture::next_parent_id = boost::get<std::string>(response[1].at("id"));
    // BOOST_TEST(boost::get<std::string>(response[1].at("is_modelelement")) == false);
}

BOOST_AUTO_TEST_CASE( test_l2_nodes ) {
    const auto parent_id = std::string("33382bc4-249b-a646-ef2b-14033605bae0");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-2->Level2-1");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 2);
    BOOST_TEST(boost::get<std::string>(response[1].at("parent_id")) == parent_id);
}

BOOST_AUTO_TEST_CASE( test_l3_nodes ) {
    const auto parent_id = std::string("0979534e-9dc1-bf0e-208a-d508862888fe");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2->Level2-1->Level3-2");
    BOOST_TEST(boost::get<int>(response[0].at("levelno")) == 3);
    BOOST_TEST(boost::get<std::string>(response[1].at("parent_id")) == parent_id);
    std::cout << boost::get<std::string>(response[1].at("id"));
    //BOOST_TEST(boost::get<bool>(response[0].at("is_modelelement")) == false;
}

BOOST_AUTO_TEST_CASE( test_l4_nodes ) {
    const auto parent_id = std::string("0b8b675f-0afb-7772-6409-81a069aedb2a");
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), parent_id}}}
    );
    auto response = TagHierarchy::Handle(query);
    BOOST_TEST(boost::get<int>(response[0].at("levelno")) == 4);
    // TODO make support for model element folders
    // assert response_l4.data[0]['type'] == NodeType.MODELELEMENTFOLDER
    // assert response_l4.data[1]['is_modelelement'] is True
    // assert response_l4.data[0]['parent_id'] == response_l3.data[1]['id']
    // assert response_l4.data[1]['parent_id'] == response_l4.data[0]['id']
}

BOOST_AUTO_TEST_SUITE_END()
