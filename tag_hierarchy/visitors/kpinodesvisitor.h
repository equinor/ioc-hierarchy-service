#pragma once

#include "models/models.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>

template<class GraphT>
class KpiNodesVisitor : public boost::default_dfs_visitor
{
public:
    explicit KpiNodesVisitor(std::set<VertexT> &valid_nodes, std::map<VertexT, std::set<VertexT>>& matched_kpis,
                                      const std::vector<std::string> &kpifilter) : valid_nodes_(valid_nodes),
                                                                                   matched_kpis_(matched_kpis),
                                                                                   path_(std::deque<VertexT>()) {}

    void discover_vertex(VertexT v, const GraphT &g)
    {
        path_.push_front(v);
        if (g[v].properties.count("is_modelelement"))
        {
            const auto is_modelelement = boost::get<bool>(g[v].properties.find("is_modelelement")->second);
            if (is_modelelement)
            {
                if (g[v].properties.find("kpigroup_id")->second.type() == typeid(std::string))
                {
                    matched_kpis_[path_[path_.size() - 2]].insert(path_[0]);
                }
                else { // KPI is null and filtered out
                    return;
                }
                for (auto const &path_part : path_)
                {
                    if (valid_nodes_.count(path_part) == 1)
                    {
                        break;
                    }
                    valid_nodes_.insert(path_part);
                }
                // I'm a valid kpi. So the kpi needs to be added to the set of models
                // that pertain to the second level of the hierarchy I am traversing
                matched_kpis_[path_[path_.size() - 2]].insert(path_[1]);
            }
        }
        return;
    }
    void finish_vertex(VertexT v, const GraphT &g) {
        path_.pop_front();
    }

private:
    std::set<VertexT> &valid_nodes_;
    std::deque<VertexT> path_;
    std::map<VertexT, std::set<VertexT>>& matched_kpis_;
};
