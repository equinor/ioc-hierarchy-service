//
// Created by Petter Moe Kvalvaag on 2019-10-04.
//

#include "tag_hierarchy/commands/filteroptions.h"

#include "tag_hierarchy/visitors/filteroptionsvisitor.h"

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

    std::function<bool(EdgeT)> edge_predicate = [&graph_](EdgeT edge) -> bool {
        return graph_[edge].id == 0;
    };
    auto filtered_graph = FilteredGraph(
            graph_, edge_predicate
    );
    const auto filter_type = boost::get<std::string>(command_map.at("type"));
    auto valid_ids = std::set<std::string>();
    std::vector<boost::default_color_type> colormap(num_vertices(graph_));
    if (filter_type == "modelowner") {
        const auto visitor = ModelOwnerFilterOptionsVisitor<FilteredGraph>(valid_ids);
        boost::depth_first_visit(filtered_graph, root_, visitor, colormap.data());
    }
    else if (filter_type == "modelclass") {
        const auto visitor = ModelClassFilterOptionsVisitor<FilteredGraph>(valid_ids);
        boost::depth_first_visit(filtered_graph, root_, visitor, colormap.data());
    }
    else {
        return {{{std::string("error"), std::string("unknown filter type")}}};
    }

    std::string return_key = filter_type + "_ids";
    return {{{
        return_key, {
            std::vector<std::string>(cbegin(valid_ids), cend(valid_ids))
        }
    }}};
}
