#include "tag_hierarchy/commands/delete.h"

namespace {
    namespace local {
        /* Recursively delete all children and all connecting edges from \param vertex */
        std::string
        DeleteVertex(TagHierarchyGraph& graph, VertexT vertex) {
            auto [vi, vi_end] = boost::adjacent_vertices(vertex, graph);
            for (auto iter = vi; vi != vi_end; vi = iter) {
                ++iter;
                DeleteVertex(graph, *vi);
            }
            boost::clear_vertex(vertex, graph);
            boost::remove_vertex(vertex, graph);
            return "Success";
        }
    }
}

// Register command with TagHierarchy
Delete del(std::string("delete"));

Delete::Delete(std::string name) : Command(name) {}

std::vector<NodeType>
Delete::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& graph = GetGraph();
    auto& vertices = GetVertices();
    auto command_map = nodes.at(0);
    // Check if graph has been initialized
    auto nodes_to_delete = boost::get<std::vector<std::string>>(command_map.at("nodes"));
    auto status = NodeType();
    for (auto const& node : nodes_to_delete) {
        if (!vertices.count(node)) {
            status[node] = "Not found";
            continue;
        }
        auto vertex = vertices[node];
        status[node] = local::DeleteVertex(graph, vertex);
    }
    return {status};
}
