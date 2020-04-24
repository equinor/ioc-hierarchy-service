#include "models/models.h"

#include <boost/test/unit_test.hpp>

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/unittests/fixture.h"

BOOST_FIXTURE_TEST_SUITE( DeleteCommandTest, Fixture );
    BOOST_AUTO_TEST_CASE( test_delete_asset )
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

    BOOST_AUTO_TEST_CASE( test_delete_model )
    {
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({"cae8efee-f222-78c1-7480-86cd49b7dc3f"})}}}
        );
        auto response = TagHierarchy::Handle(query);
        BOOST_TEST(boost::get<std::string>(response[0].at("cae8efee-f222-78c1-7480-86cd49b7dc3f")) == "Success");

        auto query2 = std::vector<NodeType>(
                {{{std::string("command"), std::string("nodes")},
                         {std::string("parentId"), std::string("be470052-4352-c1f9-e6b2-adef4dbd966e")}}}
        );
        auto response2 = TagHierarchy::Handle(query2);
        auto predicate = [](std::vector<NodeType> response2){
            auto found_node = std::find_if(response2.begin(), response2.end(), [](const NodeType& node){
                return boost::get<std::string>(node.at("name")) == "Level1-1->Level2-1->Level3-1";
            });
            return found_node == response2.end();
        };
        if (!predicate(response2)) {
            throw std::exception();
        }
    }

BOOST_AUTO_TEST_CASE( test_delete_model_twice_is_ok ) {
        auto query = std::vector<NodeType>(
                {{{std::string("command"), std::string("delete")},
                         {std::string("nodes"), std::vector<std::string>({"cae8efee-f222-78c1-7480-86cd49b7dc3f"})}}}
        );
        auto response = TagHierarchy::Handle(query);
        response = TagHierarchy::Handle(query);
        BOOST_TEST(boost::get<std::string>(response[0].at("cae8efee-f222-78c1-7480-86cd49b7dc3f")) == "Not found");
    }

BOOST_AUTO_TEST_SUITE_END()
