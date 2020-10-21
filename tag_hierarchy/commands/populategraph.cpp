//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/populategraph.h"

PopulateGraph populategraph_(std::string("populate_graph"));

PopulateGraph::PopulateGraph(std::string name) : Command(name) {}

std::vector<NodeType>
PopulateGraph::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& root_ = GetRoot();
    auto& graph_ = GetGraph();
    auto& vertices_ = GetVertices();
    auto retval = std::vector<NodeType>();
    auto const command = *nodes.begin();
    nodes.erase(nodes.begin());
    if (command.count(boost::flyweight<std::string>("add_root")) &&
        boost::get<bool>(command.at(boost::flyweight<std::string>("add_root"))))
    {
      root_ = boost::add_vertex({"root", {{boost::flyweight<std::string>("id"), "root"}, {boost::flyweight<std::string>("levelno"), 0}}}, graph_);
    }
    for (auto &node : nodes)
    {
      std::string id = boost::get<std::string>(node[boost::flyweight<std::string>("id")]);
        const auto current_vertex = boost::add_vertex({id, node}, graph_);
        if (node[boost::flyweight<std::string>("parent_id")].type() == typeid(std::string))
        {
          std::string parent_id = boost::get<std::string>(node[boost::flyweight<std::string>("parent_id")]);
            const auto parent_vertex = vertices_.find(parent_id);
            if (parent_vertex != vertices_.end())
            {
                boost::add_edge(parent_vertex->second,
                                current_vertex,
                                Connection{0},
                                graph_);
            }
        }
        else
        {
            boost::add_edge(root_,
                            current_vertex,
                            Connection{0},
                            graph_);
        }

        vertices_[id] = current_vertex;
    }
    retval.push_back({{boost::flyweight<std::string>("success"), std::string("cache populated")}});
    return retval;
}
