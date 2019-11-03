//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/populategraph.h"

REGISTER_COMMAND(PopulateGraph, populate_graph)

std::vector<NodeType>
PopulateGraph::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& root_ = GetRoot();
    auto& graph_ = GetGraph();
    auto& vertices_ = GetVertices();
    auto retval = std::vector<NodeType>();
    auto const command = *nodes.begin();
    nodes.erase(nodes.begin());
    if (command.count("add_root") &&
        boost::get<bool>(command.at("add_root")))
    {
        root_ = boost::add_vertex({"root", {{"id", "root"}, {"levelno", 0}}}, graph_);
    }
    for (auto &node : nodes)
    {
        std::string id = boost::get<std::string>(node["id"]);
        const auto current_vertex = boost::add_vertex({id, node}, graph_);
        if (node["parent_id"].type() == typeid(std::string))
        {
            std::string parent_id = boost::get<std::string>(node["parent_id"]);
            const auto parent_vertex = vertices_.find(parent_id);
            if (parent_vertex != vertices_.end())
            {
                boost::add_edge(parent_vertex->second,
                                current_vertex,
                                Connection{"physical_hierarchy"},
                                graph_);
            }
        }
        else
        {
            boost::add_edge(root_,
                            current_vertex,
                            Connection{"physical_hierarchy"},
                            graph_);
        }

        vertices_[id] = current_vertex;
    }
    retval.push_back({{std::string("success"), std::string("cache populated")}});
    return retval;
}
