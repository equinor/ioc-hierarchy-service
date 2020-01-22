//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include <tag_hierarchy/filteredhierarchyvisitor.h>
#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/nodes.h"

REGISTER_COMMAND(Nodes, nodes)

std::vector<NodeType>
Nodes::ProcessRequest(std::vector<NodeType> &nodes)
{
    auto& root_ = GetRoot();
    auto& graph_ = GetGraph();
    auto& vertices_ = GetVertices();
    auto command_map = nodes.at(0);
    auto retval = std::vector<NodeType>();
    // Check if graph has been initialized
    if (root_ == std::numeric_limits<VertexT>::max()) {
        retval.push_back({{std::string("error"), std::string("empty")}});
        return retval;
    }
    auto kpifilter = std::vector<std::string>();
    if (command_map.count("kpifilter") &&
        command_map["kpifilter"].type() == typeid(std::vector<std::string>))
    {
        kpifilter = boost::get<std::vector<std::string>>(command_map["kpifilter"]);
    }

    auto l1filter = std::vector<std::string>();
    if (command_map.count("l1filter") &&
        command_map["l1filter"].type() == typeid(std::vector<std::string>))
    {
        l1filter = boost::get<std::vector<std::string>>(command_map.at("l1filter"));
    }

    auto l2filter = std::vector<std::string>();
    if (command_map.count("l2filter") &&
        command_map["l2filter"].type() == typeid(std::vector<std::string>))
    {
        l2filter = boost::get<std::vector<std::string>>(command_map.at("l2filter"));
    }

    auto modelownerfilter = std::vector<std::string>();
    if (command_map.count("modelownerfilter") &&
        command_map["modelownerfilter"].type() == typeid(std::vector<std::string>))
    {
        modelownerfilter = boost::get<std::vector<std::string>>(command_map.at("modelownerfilter"));
    }

    auto modelclassfilter = std::vector<std::string>();
    if (command_map.count("modelclassfilter") &&
        command_map["modelclassfilter"].type() == typeid(std::vector<std::string>))
    {
        modelclassfilter = boost::get<std::vector<std::string>>(command_map.at("modelclassfilter"));
    }

    auto valid_nodes = std::set<VertexT>();
    auto valid_models = std::map<VertexT, std::set<VertexT>>();
    auto dfs_visitor = FilteredHierarchyVisitor(valid_nodes, valid_models, kpifilter);

    auto const termfunc = [l1filter, l2filter, modelownerfilter, modelclassfilter] (
            VertexT vertex, const TagHierarchyGraph& graph) {
        // Process level filter for early exit
        auto const levelno = boost::get<int>(graph[vertex].properties.at("levelno"));
        if (levelno == 1 && l1filter.size() > 0) {
            return std::find(cbegin(l1filter), cend(l1filter), graph[vertex].id) == cend(l1filter);
        }
        if (levelno == 2 && l2filter.size() > 0) {
            return std::find(cbegin(l2filter), cend(l2filter), graph[vertex].id) == cend(l2filter);
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

    auto parent_vertex = VertexT();
    if (command_map["parentId"].type() == typeid(pybind11::none))
    {
        parent_vertex = root_;
    }
    else
    {
        std::string parent_id = boost::get<std::string>(command_map["parentId"]);
        parent_vertex = vertices_[parent_id];
    }

    std::vector<boost::default_color_type> colormap(num_vertices(graph_));
    boost::depth_first_visit(graph_, parent_vertex, dfs_visitor, colormap.data(), termfunc);

    auto ei = TagHierarchyGraph::adjacency_iterator();
    auto ei_end = TagHierarchyGraph::adjacency_iterator();
    boost::tie(ei, ei_end) = boost::adjacent_vertices(parent_vertex, graph_);
    for (auto iter = ei; iter != ei_end; ++iter)
    {
        if (valid_nodes.count(*iter) == 0)
        {
            continue;
        }
        auto valid_model_ids = std::vector<std::string>();
        for (auto const& modelhierarchy : valid_models[*iter]) {
            valid_model_ids.emplace_back(graph_[modelhierarchy].id);
        }
        auto props = graph_[*iter].properties;
        props["model_ids"] = valid_model_ids;
        retval.push_back(props);
    }
    return retval;
}
