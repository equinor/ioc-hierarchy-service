//
// Created by Petter Moe Kvalvaag on 2019-10-04.
//

#include "tag_hierarchy/commands/filteroptions.h"

#include "tag_hierarchy/filteroptionsvisitor.h"

REGISTER_COMMAND(FilterOptions, filteroptions)

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

    auto l1filter = std::vector<std::string>();
    if (command_map.count("l1filter") &&
        command_map["l1filter"].type() == typeid(std::vector<std::string>))
    {
        l1filter = boost::get<std::vector<std::string>>(command_map.at("l1filter"));
    }

    auto l2filter = std::vector<std::string>();
    if (command_map.count("l2filter") &&
        command_map["l2filter"].type() == typeid(std::vector<std::string>))
    {
        l2filter = boost::get<std::vector<std::string>>(command_map.at("l2filter"));
    }

    auto valid_modelowners = std::set<std::string>();
    auto dfs_visitor = ModelOwnerFilterOptionsVisitor(valid_modelowners);

    auto const termfunc = [l1filter, l2filter] (
            VertexT vertex, const TagHierarchyGraph& graph) {
        auto const levelno = boost::get<int>(graph[vertex].properties.at("levelno"));
        if (levelno == 1 && l1filter.size() > 0) {
            return std::find(cbegin(l1filter), cend(l1filter), graph[vertex].id) == cend(l1filter);
        }
        if (levelno == 2 && l2filter.size() > 0) {
            return std::find(cbegin(l2filter), cend(l2filter), graph[vertex].id) == cend(l2filter);
        }
        return false;
    };

    std::vector<boost::default_color_type> colormap(num_vertices(graph_));
    boost::depth_first_visit(graph_, root_, dfs_visitor, colormap.data(), termfunc);

    return {{{
        std::string("modelowner_ids"), {
            std::vector<std::string>(cbegin(valid_modelowners), cend(valid_modelowners))
        }
    }}};
}
