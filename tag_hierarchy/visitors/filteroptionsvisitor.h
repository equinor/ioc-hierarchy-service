//
// Created by Petter Moe Kvalvaag on 2019-08-27.
//

#pragma once


#include "models/models.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

class FilterOptionsVisitor : public boost::default_dfs_visitor
{
public:
    explicit FilterOptionsVisitor(std::set<std::string>& valid_filter_options,
                                  std::string filter_option)
        : valid_filter_options_(valid_filter_options),
          filter_option_(filter_option){
    }
    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        if (g[v].properties.count(filter_option_) &&
            g[v].properties.find(filter_option_)->second.type() == typeid(std::string)) {
            valid_filter_options_.insert(boost::get<std::string>(g[v].properties.at(filter_option_)));
        }
    }

private:
    std::set<std::string>& valid_filter_options_;
    std::string filter_option_;
};

