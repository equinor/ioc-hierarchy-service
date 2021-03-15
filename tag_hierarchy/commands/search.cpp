//
// Created by Petter Moe Kvalvaag on 2019-10-04.
//

#include "tag_hierarchy/commands/search.h"

#include "tag_hierarchy/visitors/searchvisitor.h"
#include "tag_hierarchy/utils/exceptions.h"
#include "tag_hierarchy/utils/search.h"

#include <algorithm>
#include <regex>
#include <boost/algorithm/searching/boyer_moore.hpp>

namespace {
    namespace local {
        struct BoyerMooreSearcher : SearchUtils::Searcher {
            BoyerMooreSearcher(const std::string& search_term) : Searcher(search_term),
                                                                 searcher_(boost::algorithm::boyer_moore<std::string::const_iterator>(
                                                                         search_term_.cbegin(), search_term_.cend())){
            }
            bool HasTerm(std::string::const_iterator begin, std::string::const_iterator end) override {
                const auto not_found = std::pair<std::string::const_iterator, std::string::const_iterator>(end, end);
                return searcher_(begin, end) != not_found;
            }

            boost::algorithm::boyer_moore<std::string::const_iterator> searcher_;
        };
        struct RegExSearcher : SearchUtils::Searcher {
            RegExSearcher(const std::string& search_term) : Searcher(search_term),
                                                            searcher_(search_term){
            }
            bool HasTerm(std::string::const_iterator begin, std::string::const_iterator end) override {
                auto result = std::regex_search(begin, end, searcher_);
                std::cout << "Matching " << std::string(begin, end) << " against " << search_term_ << ": " << result << std::endl ;
                return result;
            }
            std::regex searcher_;
        };
        std::shared_ptr<SearchUtils::Searcher>
        GetSearcher(const NodeType& command_map) {
            const auto search_term = boost::get<std::string>(command_map.at("search_term"));
            const auto search_algorithm = command_map.count("search_algorithm") ?
                                          boost::get<std::string>(command_map.at("search_algorithm")) :
                                          std::string("boyer-moore");
            if (search_algorithm == "boyer-moore") {
                return std::make_shared<BoyerMooreSearcher>(search_term);
            }
            if (search_algorithm == "regex") {
                return std::make_shared<RegExSearcher>(search_term);
            }
        }
    }
}

Search search_(std::string("search"));

Search::Search(std::string name) : Command(name) {}

std::vector<NodeType>
Search::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& root_ = GetRoot();
    auto& graph_ = GetGraph();
    auto command_map = nodes.at(0);
    // Check if graph has been initialized
    if (root_ == std::numeric_limits<VertexT>::max()) {
        return {{{std::string("error"), std::string("empty")}}};
    }

    const auto searcher = local::GetSearcher(command_map);

    const auto max_results = boost::get<int>(command_map.at("max_results"));
    std::vector<boost::default_color_type> colormap(num_vertices(graph_));

    auto result_map = std::map<std::string, std::vector<VertexT>>();
    result_map["folder"] = std::vector<VertexT>();
    result_map["model"] = std::vector<VertexT>();
    result_map["modelElement"] = std::vector<VertexT>();
    auto visitor = SearchVisitor(searcher, result_map, max_results);
    try {
        auto index_map = VertexDescMap();
        boost::associative_property_map<VertexDescMap> colormap(index_map);
        boost::depth_first_visit(graph_, root_, visitor, colormap);
    }
    catch (TagHierarchyUtil::StopTraversing) {
        // It's fine, we throw to stop traversal
    }

    auto retval = std::vector<NodeType>();
    for (auto v : result_map["folder"]) {
        retval.push_back(graph_[v].properties);
    }
    for (auto v : result_map["model"]) {
        retval.push_back(graph_[v].properties);
    }
    for (auto v : result_map["modelElement"]) {
        retval.push_back(graph_[v].properties);
    }
    return retval;
}
