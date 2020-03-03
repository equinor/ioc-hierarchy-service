#include "tag_hierarchy/utils/filters.h"

std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::string>>
TagHierarchyUtils::Filters::GetFilterSettings(NodeType& command_map) {
    auto l1filter = std::vector<std::string>();
    if (command_map.count("l1filter") &&
        command_map["l1filter"].type() == typeid(std::vector<std::string>))
    {
        l1filter = boost::get<std::vector<std::string>>(command_map.at("l1filter"));
    }
    auto modelownerfilter = std::vector<std::string>();
    if (command_map.count("modelownerfilter") &&
        command_map["modelownerfilter"].type() == typeid(std::vector<std::string>))
    {
        modelownerfilter = boost::get<std::vector<std::string>>(command_map.at("modelownerfilter"));
    }
    auto modelclassfilter = std::vector<std::string>();
    if (command_map.count("modelclassfilter") &&
        command_map["modelclassfilter"].type() == typeid(std::vector<std::string>))
    {
        modelclassfilter = boost::get<std::vector<std::string>>(command_map.at("modelclassfilter"));
    }
    return {l1filter, modelownerfilter, modelclassfilter};
}

