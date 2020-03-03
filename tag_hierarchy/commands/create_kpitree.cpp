//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//


#include "tag_hierarchy/commands/create_kpitree.h"
#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/visitors/kpigroupvisitor.h"
#include "tag_hierarchy/visitors/createkpitreevisitor.h"

#include <boost/graph/filtered_graph.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

REGISTER_COMMAND(CreateKPITree, create_kpitree)

namespace {
    namespace local {
        std::vector<std::pair<std::string, std::string>>
        GetKPIGroups(const TagHierarchyGraph& graph) {
            auto kpis = std::set<std::pair<std::string, std::string>>();
            auto kpi_visitor = KPIGroupVisitor(kpis);
            boost::depth_first_search(graph, boost::visitor(kpi_visitor));
            return std::vector<std::pair<std::string, std::string>>(std::cbegin(kpis), std::cend(kpis));
        }

        std::vector<VertexT>
        GetLevel1Nodes(const TagHierarchyGraph graph, VertexT root) {
            const auto edge_predicate = std::function<bool(EdgeT)>(
                    [&graph](EdgeT edge) {
                        return graph[edge].id == 0;
                    }
            );
            const auto fg = boost::filtered_graph<TagHierarchyGraph, std::function<bool(EdgeT)>>(
                    graph, edge_predicate);
            const auto [l1_start, l1_end] = boost::adjacent_vertices(root, fg);
            auto retval = std::vector<VertexT>();
            for (auto iter = l1_start; iter != l1_end; ++iter) {
                retval.push_back(*iter);
            }
            return retval;
        }

    }
}


std::vector<NodeType>
CreateKPITree::ProcessRequest(std::vector<NodeType> &request)
{
    auto& graph = GetGraph();
    auto& kpi_map = GetEdgeLabels();
    auto& vertices = GetVertices();
    auto retval = std::vector<NodeType>();
    const auto kpi_groups = local::GetKPIGroups(graph);
    EdgeLabelMap& edge_labels = GetEdgeLabels();
    auto max_edge_label_value = std::max_element(std::cbegin(edge_labels), std::cend(edge_labels),
            [](const auto& p1, const auto& p2){
        return p1.second < p2.second;
    })->second;

    auto kpi_vertices = std::map<std::pair<VertexT, std::string>, VertexT>();
    const auto root = GetRoot();
    std::vector<VertexT> level1_nodes = local::GetLevel1Nodes(graph, root);
    for (auto kpi_group: kpi_groups) {
        edge_labels[kpi_group.second] = ++max_edge_label_value;
    }
    auto uuid_generator = boost::uuids::random_generator_mt19937();
    auto new_connections = std::set<std::tuple<VertexT, VertexT, int>>();
    for (auto level1_node: level1_nodes) {
        const auto l1_name = boost::get<std::string>(graph[level1_node].properties.at("name"));
        const auto l1_id = boost::get<std::string>(graph[level1_node].id);
        new_connections.insert({root, level1_node, 1});
        for (auto kpi_group: kpi_groups) {
            const auto node_id = boost::uuids::to_string(uuid_generator());
            const auto new_vertex = boost::add_vertex(
                    {node_id, {
                            {
                                    {std::string("id"), node_id},
                                    {std::string("parent_id"), l1_id},
                                    {std::string("is_modelelement"), false},
                                    {std::string("name"), l1_name + "-" + kpi_group.second},
                                    {std::string("levelno"), 2},
                                    {std::string("type"), std::string("folder")}
                            }}}, graph);
            vertices[node_id] = new_vertex;
            kpi_vertices[{level1_node, kpi_group.second}] = new_vertex;
            new_connections.insert({level1_node, new_vertex, 1});
        }
    }
    auto create_kpitreevisitor = CreateKPITreeVisitor(graph, new_connections, kpi_vertices);
    boost::depth_first_search(graph, boost::visitor(create_kpitreevisitor));
    std::cout << "Creating " << std::to_string(new_connections.size()) << " new connections" << std::endl;
    for (const auto new_connection: new_connections) {
        auto [src, dst, label] = new_connection;
        //const auto connection = Connection{kpi_map[label]};
        const auto connection = Connection{1};
        boost::add_edge(src, dst, connection, graph);
    }
    return retval;
}
