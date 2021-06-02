//
// Created by Petter Moe Kvalvaag on 2019-08-27.
//

#pragma once

#include "models/models.h"
#include "tag_hierarchy/utils/severity_states.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

class FilteredHierarchyVisitor : public boost::default_dfs_visitor
{
public:
    explicit FilteredHierarchyVisitor(std::set<VertexT> &valid_nodes, std::map<VertexT, std::set<VertexT>>& valid_models,
                                      const std::vector<std::string> &kpifilter, std::set<VertexT>& suppressed_nodes,
                                      std::map<VertexT, std::string> node_severity) : valid_nodes_(valid_nodes),
                                                                                  valid_models_(valid_models),
                                                                                  kpifilter_(kpifilter),
                                                                                  path_(std::deque<VertexT>()),
                                                                                  suppressed_nodes_(suppressed_nodes),
                                                                                  node_severity_(node_severity) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        path_.push_front(v);
        if (g[v].properties.count("is_modelelement"))
        {
            const auto is_modelelement = boost::get<bool>(g[v].properties.find("is_modelelement")->second);
            if (is_modelelement)
            {
                if (kpifilter_.size() != 0 &&
                    g[v].properties.count("kpigroup_id"))
                {

                    if (g[v].properties.find("kpigroup_id")->second.type() == typeid(std::string))
                    {
                        const auto kpi_id = boost::get<std::string>(g[v].properties.find("kpigroup_id")->second);
                        if (std::find(std::cbegin(kpifilter_), std::cend(kpifilter_), kpi_id) == std::cend(kpifilter_))
                        {
                            return;
                        }
                    }
                    else { // KPI is null and filtered out
                        return;
                    }
                }
                for (auto const &path_part : path_)
                {
                    if (valid_nodes_.count(path_part) == 1)
                    {
                        break;
                    }
                    valid_nodes_.insert(path_part);
                }
                // I'm a valid model element. So the model needs to be added to the set of models
                // that pertain to the second level of the hierarchy I am traversing
                valid_models_[path_[path_.size() - 2]].insert(path_[1]);
            }
        }
        return;
    }
    void finish_vertex(VertexT v, const TagHierarchyGraph &g) {
        path_.pop_front();
    }

    void finish_edge(EdgeT e, const TagHierarchyGraph &g) {
        auto target_vertex = boost::target(e, g);

        // Is the target a model element node?
        if (g[target_vertex].properties.count("is_modelelement")
            && boost::get<bool>(g[target_vertex].properties.find("is_modelelement")->second)) {
            // Yes, the target is a model element node.
            // Is this model element suppressed?
            if (g[target_vertex].properties.count("issuppressed")
                && boost::get<bool>(g[target_vertex].properties.find("issuppressed")->second)) {
                // Yes, it is suppressed. Then the source node should be suppressed as well.
                auto source_vertex = boost::source(e, g);
                suppressed_nodes_.insert(source_vertex);
                suppressed_nodes_.insert(target_vertex);
            }
            // Does this model element have a annotation severity level?
            if (g[target_vertex].properties.count("severity")) {
                // Yes, then remember this.
                node_severity_[target_vertex] = boost::get<std::string>(g[target_vertex].properties.find("severity")->second);
            }
        }
        else {
            // No, this is not a model element node.
            auto source_vertex = boost::source(e, g);
            // Is the target node suppressed?
            if (suppressed_nodes_.find(target_vertex) != suppressed_nodes_.end()) {
                // Yes, then the suppression should propagate to the source node.
                suppressed_nodes_.insert(source_vertex);
            }

            // Find and set the severity level of the node.
            auto ei = TagHierarchyGraph::adjacency_iterator();
            auto ei_end = TagHierarchyGraph::adjacency_iterator();
            boost::tie(ei, ei_end) = boost::adjacent_vertices(source_vertex, g);
            auto severity_level = -1;

            for (auto iterator = ei; iterator != ei_end; iterator++) {
                if (g[*iterator].properties.count("severity")) {
                    auto child_severity_state = boost::get<std::string>(g[target_vertex].properties.find("severity")->second);
                    auto child_severity_level = get_severity_level(child_severity_state);
                    severity_level = std::max(severity_level, child_severity_level);
                }
            }

            if (severity_level > -1) {
                node_severity_[source_vertex] = get_state(severity_level);
            }
        }
    }

private:
    std::set<VertexT> &valid_nodes_;
    std::set<VertexT> &suppressed_nodes_;
    const std::vector<std::string> &kpifilter_;
    std::deque<VertexT> path_;
    std::map<VertexT, std::set<VertexT>>& valid_models_;
    std::map<VertexT, std::string>& node_severity_;
};
