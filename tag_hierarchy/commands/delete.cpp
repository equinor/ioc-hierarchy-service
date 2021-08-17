#include "tag_hierarchy/commands/delete.h"

namespace {
    namespace local {
        std::string
        DeleteVertexRecursiveImpl(TagHierarchyGraph& graph,
                VertexT vertex,
                std::unordered_map<std::string, VertexT>& vertices,
                std::vector<VertexT>& vertices_to_delete,
                int depth) {
            depth++;
            auto [vi, vi_end] = boost::out_edges(vertex, graph);
            for (auto iter = vi; iter != vi_end; ++iter) {
                DeleteVertexRecursiveImpl(graph, boost::target(*iter, graph), vertices, vertices_to_delete, depth);
            }
            vertices_to_delete.push_back(vertex);
            if (depth==1) {
                for (auto vertex_to_delete : vertices_to_delete) {
                    const auto vertex_id = boost::get<std::string>(graph[vertex_to_delete].id);
                    boost::clear_in_edges(vertex_to_delete, graph);
                    boost::remove_vertex(vertex_to_delete, graph);
                    vertices.erase(vertex_id);
                }
            }
            return "Success";
        }

        /* Recursively delete all children and all connecting edges from \param vertex
         * Updates \param vertices which maps IDs to vertex descriptors to reflect changes */
        std::string
        DeleteVertex(TagHierarchyGraph& graph, VertexT vertex, std::unordered_map<std::string, VertexT>& vertices) {
            auto vertices_to_delete = std::vector<VertexT>();
            auto ids_to_delete = std::vector<std::string>();
            auto depth = 0;
            return DeleteVertexRecursiveImpl(graph, vertex, vertices, vertices_to_delete, depth);
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
            status[node] = std::string("Not found");
            continue;
        }
        auto vertex = vertices[node];
        status[node] = local::DeleteVertex(graph, vertex, vertices);
    }
    return {status};
}
