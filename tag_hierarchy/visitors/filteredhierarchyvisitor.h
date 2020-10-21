//
// Created by Petter Moe Kvalvaag on 2019-08-27.
//

#pragma once

#include "models/models.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

class FilteredHierarchyVisitor : public boost::default_dfs_visitor
{
public:
    explicit FilteredHierarchyVisitor(std::set<VertexT> &valid_nodes, std::map<VertexT, std::set<VertexT>>& valid_models,
                                      const std::vector<std::string> &kpifilter) : valid_nodes_(valid_nodes),
                                                                                   valid_models_(valid_models),
                                                                                   kpifilter_(kpifilter),
                                                                                   path_(std::deque<VertexT>()) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        path_.push_front(v);
        if (g[v].properties.count(boost::flyweight<std::string>("is_modelelement")))
        {
          const auto is_modelelement = boost::get<bool>(g[v].properties.find(boost::flyweight<std::string>("is_modelelement"))->second);
            if (is_modelelement)
            {
                if (kpifilter_.size() != 0 &&
                    g[v].properties.count(boost::flyweight<std::string>("kpigroup_id")))
                {

                  if (g[v].properties.find(boost::flyweight<std::string>("kpigroup_id"))->second.type() == typeid(std::string))
                    {
                      const auto kpi_id = boost::get<std::string>(g[v].properties.find(boost::flyweight<std::string>("kpigroup_id"))->second);
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

private:
    std::set<VertexT> &valid_nodes_;
    const std::vector<std::string> &kpifilter_;
    std::deque<VertexT> path_;
    std::map<VertexT, std::set<VertexT>>& valid_models_;
};
