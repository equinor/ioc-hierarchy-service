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
#include <tag_hierarchy/commands/command.h>

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

    TagHierarchyGraph& GetGraph() {
        return graph_;
    }

    std::map<std::string, VertexT>& GetVertices() {
        return vertices_;
    }

    VertexT& GetRoot() {
        return root_;
    }

    std::map<std::string, DispatchFunction> GetDispatchMap() {
        return command_func_dispatch_;
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
        auto func = command_func_dispatch_[command];
        auto retval = func(message);
        return retval; //command_func_dispatch_[command](message);
    }

    TagHierarchyImpl() : command_func_dispatch_({
                             {"store", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->Store(nodes);
                              }},
                             {"restore", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->Restore(nodes);
                              }},
                             {"flush", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  this->ClearGraph();
                                  return {{{std::string("success"), std::string("hierarchy flushed")}}};
                              }},
                             {"healthcheck", [this](std::vector<NodeType> &nodes) -> std::vector<NodeType> {
                                  return this->HealthCheck(nodes);
                              }},
                         }),
                         root_(std::numeric_limits<VertexT>::max())
    {
    };
    TagHierarchyImpl(const TagHierarchyImpl& in) : command_func_dispatch_(in.command_func_dispatch_) {}

    void Register(Command& in) {
        auto command_name = in.CommandName();
        auto func = in.Function();
        command_func_dispatch_[command_name] = func;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & graph_;
        ar & vertices_;
        ar & root_;
    }
};


std::vector<NodeType>
TagHierarchy::Handle(std::vector<NodeType>& message) {
    return GetTagHierarchy().Handle(message);
}

TagHierarchyImpl&
TagHierarchy::GetTagHierarchy() {
    static TagHierarchyImpl taghierarchy_impl;
    return taghierarchy_impl;
}

TagHierarchyGraph&
TagHierarchy::GetGraph() {
    return GetTagHierarchy().GetGraph();
}

std::map<std::string, VertexT>&
TagHierarchy::GetVertices() {
    return GetTagHierarchy().GetVertices();
}

VertexT&
TagHierarchy::GetRoot() {
    return GetTagHierarchy().GetRoot();
}

void
TagHierarchy::Register(Command &in) {
    GetTagHierarchy().Register(in);
}
