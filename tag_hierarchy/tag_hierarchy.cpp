#include "tag_hierarchy/tag_hierarchy.h"


#include "tag_hierarchy/filteredhierarchyvisitor.h"
#include "tag_hierarchy/filteroptionsvisitor.h"

#include "models/models.h"

#include <boost/graph/adj_list_serialize.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>
#include <boost/graph/graph_utility.hpp> 
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>

#include <functional>
#include <set>
#include <deque>
#include <typeinfo>

using VertexIterator = boost::graph_traits<TagHierarchyGraph>::vertex_iterator;
using InEdgeIterator = TagHierarchyGraph::in_edge_iterator;
using OutEdgeIterator = TagHierarchyGraph::out_edge_iterator;

using DispatchFunction =
    std::function<std::vector<NodeType>(std::vector<NodeType>&)>;



class TagHierarchyImpl {
private:
    TagHierarchyGraph graph_;
    std::map<std::string, VertexT> vertices_;
    std::map<std::string, DispatchFunction> command_func_dispatch_;
    VertexT root_;

public:
    std::vector<NodeType>
    PopulateGraph(std::vector<NodeType> &nodes)
    {
        auto retval = std::vector<NodeType>();
        auto const command = *nodes.begin();
        nodes.erase(nodes.begin());
        if (command.count("add_root") &&
            boost::get<bool>(command.at("add_root")))
        {
            root_ = boost::add_vertex({"root", {{"id", "root"}, {"levelno", 0}}}, graph_);
        }
        for (auto &node : nodes)
        {
            std::string id = boost::get<std::string>(node["id"]);
            const auto current_vertex = boost::add_vertex({id, node}, graph_);
            if (node["parent_id"].type() == typeid(std::string))
            {
                std::string parent_id = boost::get<std::string>(node["parent_id"]);
                const auto parent_vertex = vertices_.find(parent_id);
                if (parent_vertex != vertices_.end())
                {
                    boost::add_edge(parent_vertex->second,
                                    current_vertex,
                                    Connection{"physical_hierarchy"},
                                    graph_);
                }
            }
            else
            {
                boost::add_edge(root_,
                                current_vertex,
                                Connection{"physical_hierarchy"},
                                graph_);
            }

            vertices_[id] = current_vertex;
        }
        return retval;
    }

    std::vector<NodeType>
    Nodes(std::vector<NodeType> &nodes)
    {
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

        auto valid_nodes = std::set<VertexT>();
        auto valid_models = std::map<VertexT, std::set<VertexT>>();
        auto dfs_visitor = FilteredHierarchyVisitor(valid_nodes, valid_models, kpifilter);

        auto const termfunc = [l1filter, l2filter, modelownerfilter] (
                VertexT vertex, const TagHierarchyGraph& graph) {
            auto const levelno = boost::get<int>(graph[vertex].properties.at("levelno"));
            if (levelno == 1 && l1filter.size() > 0) {
                return std::find(cbegin(l1filter), cend(l1filter), graph[vertex].id) == cend(l1filter);
            }
            if (levelno == 2 && l2filter.size() > 0) {
                return std::find(cbegin(l2filter), cend(l2filter), graph[vertex].id) == cend(l2filter);
            }
            if (modelownerfilter.size() > 0 && graph[vertex].properties.count("modelowner")) {
                return graph[vertex].properties.at("modelowner").type() == typeid(std::string) &&
                       std::find(cbegin(modelownerfilter), cend(modelownerfilter),
                                 boost::get<std::string>(graph[vertex].properties.at("modelowner"))) ==
                       cend(modelownerfilter);
            }
            return false;
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
            auto valid_modelhierarchies = std::vector<std::string>();
            for (auto const& modelhierarchy : valid_models[*iter]) {
                valid_modelhierarchies.emplace_back(graph_[modelhierarchy].id);
            }
            auto props = graph_[*iter].properties;
            props["modelhierarchies"] = valid_modelhierarchies;
            retval.push_back(props);
        }
        return retval;
    }

    std::vector<NodeType>
    FilterOptions(std::vector<NodeType> &nodes)
    {
        auto command_map = nodes.at(0);
        // Check if graph has been initialized
        if (root_ == std::numeric_limits<VertexT>::max()) {
            return {{{std::string("error"), std::string("empty")}}};
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

        auto valid_modelowners = std::set<std::string>();
        auto dfs_visitor = ModelOwnerFilterOptionsVisitor(valid_modelowners);

        auto const termfunc = [l1filter, l2filter] (
                VertexT vertex, const TagHierarchyGraph& graph) {
            auto const levelno = boost::get<int>(graph[vertex].properties.at("levelno"));
            if (levelno == 1 && l1filter.size() > 0) {
                return std::find(cbegin(l1filter), cend(l1filter), graph[vertex].id) == cend(l1filter);
            }
            if (levelno == 2 && l2filter.size() > 0) {
                return std::find(cbegin(l2filter), cend(l2filter), graph[vertex].id) == cend(l2filter);
            }
            return false;
        };

        std::vector<boost::default_color_type> colormap(num_vertices(graph_));
        boost::depth_first_visit(graph_, root_, dfs_visitor, colormap.data(), termfunc);

        return {{{std::string("modelowner_ids"), {
            std::vector<std::string>(cbegin(valid_modelowners), cend(valid_modelowners))
        }
        }}};
    }

    std::vector<NodeType>
    Store(std::vector<NodeType>& message) {
        auto retval = std::vector<NodeType>();
        std::ostringstream stream;
        boost::archive::text_oarchive archive(stream);
        archive << *this;
        retval.push_back({{"serialized_graph", stream.str()}});
        return retval;
    }

    std::vector<NodeType>
    Restore(std::vector<NodeType>& message) {
        ClearGraph();
        auto retval = std::vector<NodeType>();
        const std::string graph_state =
            boost::get<std::string>(message[0]["serialized_hierarchy"]);
        std::istringstream buffer(graph_state);
        boost::archive::text_iarchive archive(buffer);
        archive >> *this;
        retval.push_back({{"success", true}});
        return retval;
    }

    std::vector<NodeType>
    HealthCheck(std::vector<NodeType>& message) {
        auto retval = std::vector<NodeType>();
        if (root_ == std::numeric_limits<VertexT>::max()) {
            retval.push_back({{std::string("ok"), false},
                              {std::string("error"), std::string("Cache not populated")}});
            return retval;
        }
        retval.push_back({{std::string("ok"), true}});
        return retval;
    }

    void
    ClearGraph() {
        graph_.clear();
        vertices_.clear();
        root_ = std::numeric_limits<VertexT>::max();
    }

    std::vector<NodeType>
    Handle(std::vector<NodeType> &message)
    {
        auto command_map = message.at(0);
        std::string command = boost::get<std::string>(command_map["command"]);
        return command_func_dispatch_[command](message);
    }

    TagHierarchyImpl() : command_func_dispatch_({
                             {"populate_graph", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->PopulateGraph(nodes);
                              }},
                             {"nodes", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->Nodes(nodes);
                              }},
                             {"filteroptions", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                 return this->FilterOptions(nodes);
                             }},
                             {"store", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->Store(nodes);
                              }},
                             {"restore", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->Restore(nodes);
                              }},
                             {"flush", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  this->ClearGraph();
                                  return {};
                              }},
                             {"healthcheck", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->HealthCheck(nodes);
                              }},
                         }),
                         root_(std::numeric_limits<VertexT>::max())
    {
    };
    TagHierarchyImpl(const TagHierarchyImpl& in) : command_func_dispatch_(in.command_func_dispatch_) {}

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & graph_;
        ar & vertices_;
        ar & root_;
    }
};

TagHierarchy::TagHierarchy() {
    impl_ = std::make_unique<TagHierarchyImpl>();
}

TagHierarchy::TagHierarchy(const TagHierarchy& in) :
    impl_(std::make_unique<TagHierarchyImpl>(*in.impl_))
{}

TagHierarchy::~TagHierarchy() {}

std::vector<NodeType>
TagHierarchy::Handle(std::vector<NodeType>& message) {
    return impl_->Handle(message);
}
