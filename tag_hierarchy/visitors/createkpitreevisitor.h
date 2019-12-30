#pragma once

#include "models/models.h"

#include <boost/graph/depth_first_search.hpp>

class CreateKPITreeVisitor : public boost::default_dfs_visitor
{
public:
    explicit CreateKPITreeVisitor(
            const TagHierarchyGraph& graph,
            std::set<std::tuple<VertexT, VertexT, int>>& new_connections,
            const std::map<std::pair<VertexT, std::string>, VertexT>& kpi_vertices
    ) : graph_(graph),
        new_connections_(new_connections),
        kpi_vertices_(kpi_vertices),
        path_added_(std::map<VertexT, bool>())
    {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        const auto should_connect = bool {ShouldAddToPath(graph_, v) };
        path_added_.insert({v, should_connect});
        if (should_connect) {
            path_.push_back(v);
        }
        if (g[v].properties.count("kpigroup") &&
            g[v].properties.find("kpigroup")->second.type() == typeid(std::string)) {
            auto kpigroup = boost::get<std::string>(g[v].properties.at("kpigroup"));
            auto new_path = std::vector<VertexT>();
            new_path.push_back(static_cast<VertexT>(kpi_vertices_.at({path_.at(0), kpigroup})));
            new_path.insert(end(new_path), begin(path_) + 1, end(path_));
            new_path.push_back(v);
            for (auto node = cbegin(new_path) + 1; node != cend(new_path); ++node) {
                new_connections_.insert({*(node - 1), *node, 1});
            }
        }
    }

    void finish_vertex(VertexT v, const TagHierarchyGraph &g) {
        if (path_added_[v]) {
            path_.pop_back();
        }
    }

private:
    bool ShouldAddToPath(const TagHierarchyGraph& g, VertexT v) {
        if (boost::get<int>(g[v].properties.at("levelno")) == 1) {
            return true;
        }
        if (g[v].properties.count("ismainequipment") &&
            g[v].properties.find("ismainequipment")->second.type() == typeid(bool) &&
            boost::get<bool>(g[v].properties.at("ismainequipment"))) {
            return true;
        }
        if (g[v].properties.count("type") &&
            g[v].properties.find("type")->second.type() == typeid(std::string) &&
            boost::get<std::string>(g[v].properties.at("type")) == "model") {
            return true;
        }
        return false;
    }

    const TagHierarchyGraph& graph_;
    std::set<std::tuple<VertexT, VertexT, int>>& new_connections_;
    const std::map<std::pair<VertexT, std::string>, VertexT>& kpi_vertices_;
    std::vector<VertexT> path_;
    std::map<VertexT, bool> path_added_;
};
