#include "tag_hierarchy/tag_hierarchy.h"

#include "models/models.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>
#include <boost/graph/graph_utility.hpp> 

#include <functional>
#include <typeinfo>

using TagHierarchyGraph = boost::adjacency_list<
    boost::listS, boost::vecS, boost::bidirectionalS,
    Modelhierarchy, Connection>;

using TagHierarchyT = boost::labeled_graph<TagHierarchyGraph, std::string>;

using VertexT = boost::graph_traits<TagHierarchyGraph>::vertex_descriptor;
using EdgeT = boost::graph_traits<TagHierarchyGraph>::edge_descriptor;
using VertexIterator = boost::graph_traits<TagHierarchyGraph>::vertex_iterator;

using DispatchFunction =
    std::function<std::vector<NodeType>(std::vector<NodeType>&)>;

class TagHierarchyImpl {
private:
    TagHierarchyGraph graph_;
    std::map<std::string, VertexT> vertices_;
    std::map<std::string, DispatchFunction> command_func_dispatch_;

public:

    std::vector<NodeType>
    PopulateGraph(std::vector<NodeType>& nodes) {
        auto retval = std::vector<NodeType>();
        nodes.erase(nodes.begin());
        for (auto & node : nodes) {
            std::string id = boost::get<std::string>(node["id"]);
            const auto current_vertex = boost::add_vertex({id, node}, graph_);
            std::cout << "Adding node " << id;
            if (node["parent_id"].which() != 2)
            {
                std::string parent_id = boost::get<std::string>(node["parent_id"]);
                const auto parent_vertex = vertices_.find(parent_id);
                if (parent_vertex != vertices_.end())
                {
                    boost::add_edge(parent_vertex->second,
                                    current_vertex,
                                    Connection{"physical_hierarchy"},
                                    graph_);
                    std::cout << " with parent id " << parent_id << std::endl;
                }
                else {
                    std::cout << " which does not have a parent" << std::endl;
                }
            }
            vertices_[id] = current_vertex;
        }
        for (auto vd : boost::make_iterator_range(boost::vertices(graph_))) {
            std::cout << graph_[vd].id << std::endl;
        }
        boost::write_graphviz(std::cout, graph_,
                              [&](auto &out, auto v) {
                                  out << "[id=\"" << graph_[v].id << "\"]";
                                  if (graph_[v].properties["parent_id"].which() == 3)
                                  {
                                    std::string parent_id = boost::get<std::string>(graph_[v].properties["parent_id"]);
                                    out << "[parent_id=\"" << parent_id << "\"]";
                                  }
                                 // out << "[name=\"" << tag_hierarchy["b"].name << "\"]" << std::endl;
                                 // out << "[parent_id=\"" << tag_hierarchy["b"].parent_id << "\"]" << std::endl;
                                  } /*,
                              [&](auto &out, auto e) {
                                  out << "[type=\"" << tag_hierarchy["b"].type << "\"]";
                              }*/);
        return retval;
    }

    std::vector<NodeType>
    Nodes(std::vector<NodeType>& nodes) {
        auto command_map = nodes.at(0);
        auto retval = std::vector<NodeType>();
        if (command_map["parentId"].type() == typeid(pybind11::none)) {
            for (auto [vi, vi_end] = vertices(graph_); vi != vi_end; ++vi)
            {
                auto [ei, ei_end] = boost::in_edges(*vi, this->graph_);
                if (ei == ei_end)
                {
                    retval.push_back(graph_[*vi].properties);
                    std::cout << "ID " << graph_[*vi].id << " has no parent" << std::endl;
                }
            }
            return retval;
        }
        else {
            std::string parent_id = boost::get<std::string>(command_map["parentId"]);
            auto [ei, ei_end] = boost::adjacent_vertices(vertices_[parent_id], this->graph_);
            for (auto iter = ei; iter != ei_end; ++iter) {
                retval.push_back(graph_[*iter].properties);
                std::cout << "Node " << graph_[*iter].id << " has parent " << parent_id
                    << std::endl;
            }
        }

        return retval;
    }

    std::vector<NodeType>
    Handle(std::vector<NodeType> &message)
    {
        auto command_map = message.at(0);
        std::string command = boost::get<std::string>(command_map["command"]);
        return command_func_dispatch_[command](message);
    }

    TagHierarchyImpl() {
        command_func_dispatch_ = {
            {
                "populate_graph", [this](std::vector<NodeType>& nodes) -> std::vector<NodeType> {
                    return this->PopulateGraph(nodes);
                }
            },
            {
                "nodes", [this](std::vector<NodeType>& nodes) -> std::vector<NodeType> {
                    return this->Nodes(nodes);
                }
            },
        };
    }
};

TagHierarchy::TagHierarchy() {
    impl_ = std::make_unique<TagHierarchyImpl>();
}

TagHierarchy::~TagHierarchy() {}

std::vector<NodeType>
TagHierarchy::Handle(std::vector<NodeType>& message) {
    return impl_->Handle(message);
}
