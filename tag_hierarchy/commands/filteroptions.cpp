//
// Created by Petter Moe Kvalvaag on 2019-10-04.
//

#include "tag_hierarchy/commands/filteroptions.h"

#include "tag_hierarchy/visitors/filteroptionsvisitor.h"

FilterOptions filteroptions(std::string("filteroptions"));

FilterOptions::FilterOptions(std::string name) : Command(name) {}

std::vector<NodeType>
FilterOptions::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& root_ = GetRoot();
    auto& graph_ = GetGraph();
    auto command_map = nodes.at(0);
    // Check if graph has been initialized
    if (root_ == std::numeric_limits<VertexT>::max()) {
        return {{{std::string("error"), std::string("empty")}}};
    }

    auto index_map = VertexDescMap();
    boost::associative_property_map<VertexDescMap> colormap(index_map);
    const auto filter_type = boost::get<std::string>(command_map.at("type"));
    auto valid_ids = std::set<std::string>();
    const auto visitor = FilterOptionsVisitor(valid_ids, filter_type);
    boost::depth_first_visit(graph_, root_, visitor, colormap);

    std::string return_key = filter_type + "_ids";
    return {{{
        return_key, {
            std::vector<std::string>(cbegin(valid_ids), cend(valid_ids))
        }
    }}};
}
