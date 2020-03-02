//
// Created by Petter Moe Kvalvaag on 2019-10-04.
//

#include "tag_hierarchy/commands/search.h"

#include "tag_hierarchy/visitors/searchvisitor.h"
#include "tag_hierarchy/utils/exceptions.h"

#include <boost/algorithm/searching/boyer_moore.hpp>

namespace {
    namespace local {
        std::function<bool(std::string::const_iterator, std::string::const_iterator)>
        GetSearcher(const NodeType& command_map) {
            const auto search_term = boost::get<std::string>(command_map.at("search_term"));
            const auto search_algorithm = command_map.count("search_algorithm") ?
                                          boost::get<std::string>(command_map.at("search_algorithm")) :
                                          std::string("boyer-moore");
            if (search_algorithm == "boyer-moore") {
                return [search_term] (std::string::const_iterator begin, std::string::const_iterator end) {
                    static boost::algorithm::boyer_moore<std::string::const_iterator> searcher(
                            search_term.cbegin(), search_term.cend()
                    );
                    const auto not_found = std::pair<std::string::const_iterator, std::string::const_iterator>(end, end);
                    if (searcher(begin, end) != not_found) {
                        return true;
                    }
                    return false;
                };
            }
        }
    }
}

REGISTER_COMMAND(Search, search)

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
        boost::depth_first_visit(graph_, root_, visitor, colormap.data());
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
