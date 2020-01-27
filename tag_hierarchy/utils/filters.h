#include "models/models.h"

namespace TagHierarchyUtils {
    namespace Filters {
        std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::string>>
        GetFilterSettings(NodeType& command_map);

        template<typename GraphT>
        std::function<bool(VertexT vertex, const GraphT& graph)>
        GetTermfunc(NodeType& command_map) {
            auto [l1filter, modelownerfilter, modelclassfilter] = TagHierarchyUtils::Filters::GetFilterSettings(command_map);
            return [l1filter=l1filter, modelownerfilter=modelownerfilter, modelclassfilter=modelclassfilter] (
                    VertexT vertex, const GraphT& graph) {
                auto const levelno = boost::get<int>(graph[vertex].properties.at("levelno"));
                if (levelno == 1 && l1filter.size() > 0) {
                    return std::find(cbegin(l1filter), cend(l1filter), graph[vertex].id) == cend(l1filter);
                }
                if (modelownerfilter.size() > 0 && graph[vertex].properties.count("modelowner")) {
                    return graph[vertex].properties.at("modelowner").type() == typeid(std::string) &&
                           std::find(cbegin(modelownerfilter), cend(modelownerfilter),
                                     boost::get<std::string>(graph[vertex].properties.at("modelowner"))) ==
                           cend(modelownerfilter);
                }
                if (modelclassfilter.size() > 0 && graph[vertex].properties.count("modelclass")) {
                    return graph[vertex].properties.at("modelclass").type() == typeid(std::string) &&
                           std::find(cbegin(modelclassfilter), cend(modelclassfilter),
                                     boost::get<std::string>(graph[vertex].properties.at("modelclass"))) ==
                           cend(modelclassfilter);
                }
                return false;
            };
        }
    }
}

