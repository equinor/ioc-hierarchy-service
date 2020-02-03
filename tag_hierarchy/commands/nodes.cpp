//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include "tag_hierarchy/commands/nodes.h"

#include "tag_hierarchy/visitors/filteredhierarchyvisitor.h"
#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/utils/filters.h"

REGISTER_COMMAND(Nodes, nodes)

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

    auto valid_nodes = std::set<VertexT>();
    auto valid_models = std::map<VertexT, std::set<VertexT>>();
    auto kpifilter = std::vector<std::string>();
    std::function<bool(EdgeT)> edge_predicate = [&graph_](EdgeT edge) -> bool {
        return graph_[edge].id == 0;
    };
    auto filtered_graph = FilteredGraph(
            graph_, edge_predicate
    );
    auto dfs_visitor = FilteredHierarchyVisitor<FilteredGraph>(valid_nodes, valid_models, kpifilter);

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

    std::vector<boost::default_color_type> colormap(num_vertices(graph_));
    const auto termfunc = TagHierarchyUtils::Filters::GetTermfunc<FilteredGraph >(command_map);
    boost::depth_first_visit(filtered_graph, parent_vertex, dfs_visitor, colormap.data(), termfunc);

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
        retval.push_back(props);
    }
    return retval;
}
