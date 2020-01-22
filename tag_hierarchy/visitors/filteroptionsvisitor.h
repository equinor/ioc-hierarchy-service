//
// Created by Petter Moe Kvalvaag on 2019-08-27.
//

#pragma once


#include "models/models.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

class ModelOwnerFilterOptionsVisitor : public boost::default_dfs_visitor
{
public:
    explicit ModelOwnerFilterOptionsVisitor(
            std::set<std::string>& valid_modelowners
    ) : valid_modelowners_(valid_modelowners) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        if (g[v].properties.count("modelowner") &&
            g[v].properties.find("modelowner")->second.type() == typeid(std::string)) {
            valid_modelowners_.insert(boost::get<std::string>(
                    g[v].properties.at("modelowner")
            ));
        }
    }

private:
    std::set<std::string>& valid_modelowners_;
};

class ModelClassFilterOptionsVisitor : public boost::default_dfs_visitor
{
public:
    explicit ModelClassFilterOptionsVisitor(
            std::set<std::string>& valid_modelclasses
    ) : valid_modelclasses_(valid_modelclasses) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        if (g[v].properties.count("modelclass") &&
            g[v].properties.find("modelclass")->second.type() == typeid(std::string)) {
            valid_modelclasses_.insert(boost::get<std::string>(
                    g[v].properties.at("modelclass")
            ));
        }
    }

private:
    std::set<std::string>& valid_modelclasses_;
};
