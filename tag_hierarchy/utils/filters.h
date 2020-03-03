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
                // If we are not a model, stop here
                if (graph[vertex].properties.count("type") &&
                    boost::get<std::string>(graph[vertex].properties.at("type")) != "model") {
                    return false;
                }
                auto model_is_valid = true;
                if (modelownerfilter.size() > 0 && graph[vertex].properties.count("modelowner")) {
                    if (graph[vertex].properties.at("modelowner").type() == typeid(pybind11::none)) {
                        model_is_valid = false;
                    }
                    else {
                        model_is_valid &=
                                std::find(cbegin(modelownerfilter), cend(modelownerfilter),
                                          boost::get<std::string>(graph[vertex].properties.at("modelowner"))) !=
                                cend(modelownerfilter);
                    }
                }
                if (modelclassfilter.size() > 0 && graph[vertex].properties.count("modelclass")) {
                    if (graph[vertex].properties.at("modelclass").type() == typeid(pybind11::none)) {
                        model_is_valid = false;
                    }
                    else {
                        model_is_valid &=
                                std::find(cbegin(modelclassfilter), cend(modelclassfilter),
                                          boost::get<std::string>(graph[vertex].properties.at("modelclass"))) !=
                                cend(modelclassfilter);
                    }
                }
                return !model_is_valid;
            };
        }
    }
}

