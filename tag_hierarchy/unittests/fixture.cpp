#include <fstream>
#include "tag_hierarchy/unittests/fixture.h"

#include <boost/test/unit_test.hpp>
#include <boost/json.hpp>

#include "models/models.h"
#include "tag_hierarchy/tag_hierarchy.h"


Fixture::Fixture() {
    BOOST_TEST_MESSAGE( "Constructing test fixture" );
    std::ifstream ifs("hierarchy_dump.json");
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    boost::json::array modelhierarchy_json = boost::json::parse(content).as_array();
    auto modelhierarchy = std::vector<NodeType>{};
    for (const auto& value : modelhierarchy_json) {
        auto temp = NodeType{};
        const auto object = boost::json::value_to<boost::json::object>(value);

        for (auto iter = object.cbegin(); iter != object.cend(); ++iter) {
            auto value = iter->value();
            if (value.is_null()) {
                temp.insert({std::string(iter->key()), pybind11::none()});
            }
            else if (value.is_string()) {
                temp.insert({std::string(iter->key()), boost::json::value_to<std::string>(iter->value())});
            }
            else if (value.is_double()) {
                temp.insert({std::string(iter->key()), boost::json::value_to<double>(iter->value())});
            }
            else if (value.is_bool()) {
                temp.insert({std::string(iter->key()), boost::json::value_to<bool>(iter->value())});
            }
            else if (value.is_int64()) {
                temp.insert({std::string(iter->key()), boost::json::value_to<int>(iter->value())});
            }
        }
        modelhierarchy.push_back(temp);
    }
    
    TagHierarchy::Handle(modelhierarchy);
}

Fixture::~Fixture() {
    BOOST_TEST_MESSAGE( "teardown fixture" );
    auto flush_command = std::vector<NodeType>(
                                               {{{std::string("command"), std::string("flush")}}}
                                               );
    TagHierarchy::Handle(flush_command);
}
