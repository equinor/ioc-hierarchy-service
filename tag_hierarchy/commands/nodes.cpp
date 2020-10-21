//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include <tag_hierarchy/visitors/filteredhierarchyvisitor.h>
#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/nodes.h"

Nodes nodes(std::string("nodes"));

Nodes::Nodes(std::string name) : Command(name) {}

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
      retval.push_back({{boost::flyweight<std::string>("error"), std::string("empty")}});
        return retval;
    }
    auto kpifilter = std::vector<std::string>();
    if (command_map.count(boost::flyweight<std::string>("kpifilter")) &&
        command_map[boost::flyweight<std::string>("kpifilter")].type() == typeid(std::vector<std::string>))
    {
      kpifilter = boost::get<std::vector<std::string>>(command_map[boost::flyweight<std::string>("kpifilter")]);
    }

    auto l1filter = std::vector<std::string>();
    if (command_map.count(boost::flyweight<std::string>("l1filter")) &&
        command_map[boost::flyweight<std::string>("l1filter")].type() == typeid(std::vector<std::string>))
    {
      l1filter = boost::get<std::vector<std::string>>(command_map.at(boost::flyweight<std::string>("l1filter")));
    }

    auto l2filter = std::vector<std::string>();
      if (command_map.count(boost::flyweight<std::string>("l2filter")) &&
          command_map[boost::flyweight<std::string>("l2filter")].type() == typeid(std::vector<std::string>))
    {
      l2filter = boost::get<std::vector<std::string>>(command_map.at(boost::flyweight<std::string>("l2filter")));
    }

    auto modelownerfilter = std::vector<std::string>();
      if (command_map.count(boost::flyweight<std::string>("modelownerfilter")) &&
          command_map[boost::flyweight<std::string>("modelownerfilter")].type() == typeid(std::vector<std::string>))
    {
      modelownerfilter = boost::get<std::vector<std::string>>(command_map.at(boost::flyweight<std::string>("modelownerfilter")));
    }

    auto modelclassfilter = std::vector<std::string>();
      if (command_map.count(boost::flyweight<std::string>("modelclassfilter")) &&
          command_map[boost::flyweight<std::string>("modelclassfilter")].type() == typeid(std::vector<std::string>))
    {
      modelclassfilter = boost::get<std::vector<std::string>>(command_map.at(boost::flyweight<std::string>("modelclassfilter")));
    }

    auto valid_nodes = std::set<VertexT>();
    auto valid_models = std::map<VertexT, std::set<VertexT>>();
    auto dfs_visitor = FilteredHierarchyVisitor(valid_nodes, valid_models, kpifilter);

    auto const termfunc = [l1filter, l2filter, modelownerfilter, modelclassfilter] (
            VertexT vertex, const TagHierarchyGraph& graph) {
        // Process level filter for early exit
                            auto const levelno = boost::get<int>(graph[vertex].properties.at(boost::flyweight<std::string>("levelno")));
        if (levelno == 1 && l1filter.size() > 0) {
            return std::find(cbegin(l1filter), cend(l1filter), graph[vertex].id) == cend(l1filter);
        }
        if (levelno == 2 && l2filter.size() > 0) {
            return std::find(cbegin(l2filter), cend(l2filter), graph[vertex].id) == cend(l2filter);
        }
        // If we are not a model, stop here
        if (graph[vertex].properties.count(boost::flyweight<std::string>("type")) &&
            boost::get<std::string>(graph[vertex].properties.at(boost::flyweight<std::string>("type"))) != "model") {
            return false;
        }
        auto model_is_valid = true;
        if (modelownerfilter.size() > 0 && graph[vertex].properties.count(boost::flyweight<std::string>("modelowner"))) {
          if (graph[vertex].properties.at(boost::flyweight<std::string>("modelowner")).type() == typeid(pybind11::none)) {
                model_is_valid = false;
            }
            else {
                model_is_valid &=
                        std::find(cbegin(modelownerfilter), cend(modelownerfilter),
                                  boost::get<std::string>(graph[vertex].properties.at(boost::flyweight<std::string>("modelowner")))) !=
                        cend(modelownerfilter);
            }
        }
            if (modelclassfilter.size() > 0 && graph[vertex].properties.count(boost::flyweight<std::string>("modelclass"))) {
              if (graph[vertex].properties.at(boost::flyweight<std::string>("modelclass")).type() == typeid(pybind11::none)) {
                model_is_valid = false;
            }
            else {
                model_is_valid &=
                        std::find(cbegin(modelclassfilter), cend(modelclassfilter),
                                  boost::get<std::string>(graph[vertex].properties.at(boost::flyweight<std::string>("modelclass")))) !=
                        cend(modelclassfilter);
            }
        }
        return !model_is_valid;
    };

    auto parent_vertex = VertexT();
    if (command_map[boost::flyweight<std::string>("parentId")].type() == typeid(pybind11::none))
    {
        parent_vertex = root_;
    }
    else
    {
      std::string parent_id = boost::get<std::string>(command_map[boost::flyweight<std::string>("parentId")]);
        parent_vertex = vertices_[parent_id];
    }

    auto index_map = VertexDescMap();
    boost::associative_property_map<VertexDescMap> colormap(index_map);
    boost::depth_first_visit(graph_, parent_vertex, dfs_visitor, colormap, termfunc);

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
        props[boost::flyweight<std::string>("model_ids")] = valid_model_ids;
        retval.push_back(props);
    }
    return retval;
}
