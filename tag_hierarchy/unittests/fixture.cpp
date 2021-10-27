#include <fstream>
#include "tag_hierarchy/unittests/fixture.h"

#include <boost/test/unit_test.hpp>
#include <nlohmann/json.hpp>

#include "models/models.h"
#include "tag_hierarchy/tag_hierarchy.h"


Fixture::Fixture() {
    BOOST_TEST_MESSAGE( "Constructing test fixture" );
    std::ifstream ifs("hierarchy_dump.json");
    nlohmann::json modelhierarchy_json;
    ifs >> modelhierarchy_json;

    auto modelhierarchy = std::vector<NodeType>{};
    for (const auto value : modelhierarchy_json) {
        auto temp = NodeType{};
        const auto object = value;

        for (auto& element : value.items()) {
            auto value = element.value();
            if (value.is_null()) {
                temp.insert({std::string(element.key()), pybind11::none()});
            }
            else if (value.is_string()) {
                temp.insert({std::string(element.key()), value.get<std::string>()});
            }
            else if (value.is_number_float()) {
                temp.insert({std::string(element.key()), value.get<double>()});
            }
            else if (value.is_boolean()) {
                temp.insert({std::string(element.key()), value.get<bool>()});
            }
            else if (value.is_number_integer()) {
                temp.insert({std::string(element.key()), value.get<int>()});
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
