#define BOOST_TEST_MODULE tag_hierarchy
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "models/models.h"

#include "tag_hierarchy/tag_hierarchy.h"

struct Fixture {
  Fixture() : tag_hierarchy(TagHierarchy()) {
     BOOST_TEST_MESSAGE( "population graph" );
     auto modelhierarchy = std::vector<NodeType>(
     #include "hierarchy_dump.cpp"
     );
     tag_hierarchy.Handle(modelhierarchy);
  }

  ~Fixture() {
      BOOST_TEST_MESSAGE( "teardown fixture" );
  }

  TagHierarchy tag_hierarchy;
};

BOOST_FIXTURE_TEST_SUITE(tag_hierarchy_test, Fixture)
BOOST_AUTO_TEST_CASE( test_nodes )
{
    auto query = std::vector<NodeType>(
        {{{std::string("command"), std::string("nodes")},
          {std::string("parentId"), pybind11::none()}}}
    );
    auto response = tag_hierarchy.Handle(query);
    BOOST_TEST(boost::get<std::string>(response[0].at("name")) == "Level1-1");
    BOOST_TEST(boost::get<std::string>(response[1].at("name")) == "Level1-2");
    BOOST_TEST(boost::get<int>(response[1].at("levelno")) == 1);
    BOOST_TEST(boost::get<pybind11::none>(response[1].at("parent_id")) == pybind11::none());
    // BOOST_TEST(boost::get<std::string>(response[1].at("is_modelelement")) == false);
}
BOOST_AUTO_TEST_CASE( test_l2_nodes ) {

}
BOOST_AUTO_TEST_SUITE_END()
/*
        params = {
            'parentId': response.data[1]['id'],
        }
        response_l2 = client.get('/nodes/', params, content_type='application/json')

        assert response_l2.data[0]['name'] == 'Level1-2->Level2-1'
        assert response_l2.data[1]['levelno'] == 2
        assert response_l2.data[1]['parent_id'] == response.data[1]['id']

        params = {
            'parentId': response_l2.data[0]['id'],
        }

        response_l3 = client.get('/nodes/', params, content_type='application/json')

        assert response_l3.data[1]['name'] == 'Level1-2->Level2-1->Level3-2'
        assert response_l3.data[0]['levelno'] == 3
        assert response_l3.data[1]['parent_id'] == response_l2.data[0]['id']
        assert response_l3.data[0]['is_modelelement'] is False

        params = {
            'parentId': response_l3.data[1]['id'],
            'modelElementsFolder': True
        }
        response_l4 = client.get('/nodes/', params, content_type='application/json')

        assert response_l4.data[0]['levelno'] == 4
        assert response_l4.data[0]['type'] == NodeType.MODELELEMENTFOLDER
        assert response_l4.data[1]['is_modelelement'] is True
        assert response_l4.data[0]['parent_id'] == response_l3.data[1]['id']
        assert response_l4.data[1]['parent_id'] == response_l4.data[0]['id']
 */
