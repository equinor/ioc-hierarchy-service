#include "tag_hierarchy/commands/update.h"


#include <algorithm>
#include <regex>
#include <boost/algorithm/searching/boyer_moore.hpp>

Update update_(std::string("update"));

Update::Update(std::string name) : Command(name) {}

std::vector<NodeType>
Update::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& graph = GetGraph();
    auto& vertices = GetVertices();
    auto const command = *nodes.begin();
    nodes.erase(nodes.begin());
    auto retval = std::vector<NodeType>();

    for (const auto node: nodes) {
        auto existing_node = vertices.at(boost::get<std::string>(node.at("id")));
        // Verify that the updated object has the same parent ID, otherwise reject
        if (graph[existing_node].properties.at("parent_id") != node.at("parent_id")) {
            std::string error_message = "Tried to modify parent id of object " +
                                        boost::get<std::string>(node.at("id"));
            retval.push_back({{{std::string("error"), error_message}}});
        }
        else {
            graph[existing_node].properties = node;
        }
    }
    if (retval.size() > 0) {
        retval.insert(retval.begin(),
                      {{{std::string("error"), std::string("conflicting updates")}}});
    }
    else {
        retval.push_back({{{std::string("ok"), std::string("true")}}});
    }
    return retval;
}
