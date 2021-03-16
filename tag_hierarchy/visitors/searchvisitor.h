#pragma once

#include "models/models.h"
#include "tag_hierarchy/utils/exceptions.h"
#include "tag_hierarchy/utils/search.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/algorithm/searching/boyer_moore.hpp>

class SearchVisitor : public boost::default_dfs_visitor
{
public:
    explicit SearchVisitor(
            std::shared_ptr<SearchUtils::Searcher> searcher,
            std::vector<VertexT>& hits,
            const std::vector<std::string>& search_keys, int max_results
    ) : searcher_(searcher),
    hits_(hits),
    search_keys_(search_keys),
    max_results_(max_results),
    no_results_(0) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        if (no_results_ >= max_results_) {
            throw TagHierarchyUtil::StopTraversing();
        }

        auto corpi = std::vector<std::string>();
        for (const auto search_key : search_keys_) {
            if (g[v].properties.count(search_key) && g[v].properties.at(search_key).type() == typeid(std::string)) {
                corpi.push_back(boost::get<std::string>(g[v].properties.at(search_key)));
            }
        }
        auto has_match = false;
        for (const auto corpus : corpi) {
            has_match |= searcher_->HasTerm(corpus.begin(), corpus.end());
        }
        if (has_match) {
            hits_.push_back(v);
            no_results_++;
        }
    }

private:
    std::shared_ptr<SearchUtils::Searcher> searcher_;
    std::vector<VertexT>& hits_;
    const std::vector<std::string>& search_keys_;
    int max_results_;
    int no_results_;
};
