#pragma once

#include "models/models.h"

#include <boost/graph/depth_first_search.hpp>

class KPIGroupVisitor : public boost::default_dfs_visitor
{
public:
    explicit KPIGroupVisitor(
            std::set<std::pair<std::string, std::string>>& existing_kpi_groups
    ) : kpi_groups_(existing_kpi_groups) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        if (g[v].properties.count("kpigroup") &&
            g[v].properties.find("kpigroup")->second.type() == typeid(std::string)) {
            kpi_groups_.insert(
                {
                    boost::get<std::string>(g[v].properties.at("kpigroup_id")),
                    boost::get<std::string>(g[v].properties.at("kpigroup"))
                }
            );
        }
    }

private:
    std::set<std::pair<std::string, std::string>>& kpi_groups_;
};
