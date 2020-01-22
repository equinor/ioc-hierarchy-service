#pragma once

#include "models/models.h"
#include "tag_hierarchy/utils/exceptions.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/algorithm/searching/boyer_moore.hpp>

class SearchVisitor : public boost::default_dfs_visitor
{
public:
    explicit SearchVisitor(
            boost::algorithm::boyer_moore<std::string::const_iterator>& searcher,
            std::map<std::string, std::vector<VertexT>>& hits,
            int max_results
    ) : searcher_(searcher), hits_(hits), max_results_(max_results), no_results_(0) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        if (no_results_ > max_results_) {
            throw TagHierarchyUtil::StopTraversing();
        }

        if (g[v].properties.count("type") == 0) {
            return;
        }
        const auto node_type = boost::get<std::string>(g[v].properties.at("type"));
        if (node_type == "folder" ||
            node_type == "model") {
            if (g[v].properties.count("name") == 0 || g[v].properties.at("name").type() != typeid(std::string)) {
                return;
            }
            auto corpus = boost::get<std::string>(g[v].properties.at("name"));
            const auto not_found = std::pair<std::string::const_iterator, std::string::const_iterator>(corpus.end(),
                    corpus.end());
            if (searcher_(corpus.cbegin(), corpus.cend()) != not_found) {
                hits_[node_type].push_back(v);
                no_results_++;
            }
        }
        else if (node_type == "modelElement") {
            auto corpi = std::vector<std::string>();
            if (g[v].properties.count("name") == 1 && g[v].properties.at("name").type() == typeid(std::string)) {
                corpi.push_back(boost::get<std::string>(g[v].properties.at("name")));
            }
            if (g[v].properties.count("tag") && g[v].properties.at("tag").type() == typeid(std::string)) {
                corpi.push_back(boost::get<std::string>(g[v].properties.at("tag")));
            }
            auto has_match = false;
            for (const auto corpus : corpi) {
                const auto not_found = std::pair<std::string::const_iterator, std::string::const_iterator>(corpus.end(),
                                                                                                           corpus.end());
                if (searcher_(corpus.begin(), corpus.end()) != not_found) {
                    has_match = true;
                }
            }
            if (has_match) {
                hits_[node_type].push_back(v);
                no_results_++;
            }
        }
    }

private:
    boost::algorithm::boyer_moore<std::string::const_iterator> searcher_;
    std::map<std::string, std::vector<VertexT>>& hits_;
    int max_results_;
    int no_results_;
};
