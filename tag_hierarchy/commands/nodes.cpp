//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include <tag_hierarchy/visitors/filteredhierarchyvisitor.h>
#include <tag_hierarchy/utils/filters.h>
#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/nodes.h"

Nodes nodes(std::string("nodes"));

Nodes::Nodes(std::string name) : Command(name) {}

std::vector<NodeType>
Nodes::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& root_ = GetRoot();
    auto& graph_ = GetGraph();
    auto& vertices_ = GetVertices();
    auto command_map = nodes.at(0);
    auto retval = std::vector<NodeType>();
    // Check if graph has been initialized
    if (root_ == std::numeric_limits<VertexT>::max()) {
        retval.push_back({{std::string("error"), std::string("empty")}});
        return retval;
    }
    auto kpifilter = std::vector<std::string>();
    if (command_map.count("kpifilter") &&
        command_map["kpifilter"].type() == typeid(std::vector<std::string>))
    {
        kpifilter = boost::get<std::vector<std::string>>(command_map["kpifilter"]);
    }

    auto valid_nodes = std::set<VertexT>();
    auto valid_models = std::unordered_map<VertexT, std::set<VertexT>>();
    auto suppressed_nodes = std::set<VertexT>();
    auto node_severity = std::unordered_map<VertexT, int>();
    auto data_access_level = std::unordered_map<VertexT, std::string>();
    auto dfs_visitor = FilteredHierarchyVisitor(valid_nodes, valid_models, kpifilter, suppressed_nodes, node_severity, data_access_level);

    const auto termfunc = TagHierarchyUtils::Filters::GetTermfunc<TagHierarchyGraph>(command_map);

    auto parent_vertex = VertexT();
    if (command_map["parentId"].type() == typeid(pybind11::none))
    {
        parent_vertex = root_;
    }
    else
    {
        std::string parent_id = boost::get<std::string>(command_map["parentId"]);
        parent_vertex = vertices_[parent_id];
    }

    auto index_map = VertexDescMap();
    boost::associative_property_map<VertexDescMap> colormap(index_map);
    boost::depth_first_visit(graph_, parent_vertex, dfs_visitor, colormap, termfunc);

    auto ei = TagHierarchyGraph::adjacency_iterator();
    auto ei_end = TagHierarchyGraph::adjacency_iterator();
    boost::tie(ei, ei_end) = boost::adjacent_vertices(parent_vertex, graph_);
    for (auto iter = ei; iter != ei_end; ++iter)
    {
        if (valid_nodes.count(*iter) == 0)
        {
            continue;
        }
        auto valid_model_ids = std::vector<std::string>();
        for (auto const& modelhierarchy : valid_models[*iter]) {
            valid_model_ids.emplace_back(graph_[modelhierarchy].id);
        }
        auto props = graph_[*iter].properties;
        props["model_ids"] = valid_model_ids;

        // Write suppression property to the nodes of the graph.
        props["issuppressed"] = (suppressed_nodes.find(*iter) != suppressed_nodes.end());

        if (node_severity.find(*iter) != node_severity.end()) {
            props["severity"] = node_severity[*iter];
        }

        if (data_access_level.find(*iter) != data_access_level.end()) {
            props["dataaccesslevel"] = data_access_level[*iter];
        }

        retval.push_back(props);
    }
    return retval;
}
