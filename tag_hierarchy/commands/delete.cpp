#include "tag_hierarchy/commands/delete.h"

namespace {
    namespace local {
        /* Recursively delete all children and all connecting edges from \param vertex */
        std::string
        DeleteVertex(TagHierarchyGraph& graph, VertexT vertex, std::vector<VertexT> vertices_to_delete = {}, int depth=0) {
            depth++;
            auto [vi, vi_end] = boost::adjacent_vertices(vertex, graph);
            auto next = decltype(vi)();
            for (next = vi; vi != vi_end; vi = next) {
                ++next;
                DeleteVertex(graph, *vi, vertices_to_delete, depth);
            }
            vertices_to_delete.push_back(vertex);
            if (depth==1) {
                for (auto vertex_to_delete : vertices_to_delete) {
                    boost::clear_in_edges(vertex_to_delete, graph);
                    boost::remove_vertex(vertex_to_delete, graph);
                }
            }
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
