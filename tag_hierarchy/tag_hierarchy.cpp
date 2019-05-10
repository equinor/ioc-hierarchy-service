#include "tag_hierarchy/tag_hierarchy.h"
#include "models/models.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>

using TagHierarchyGraph = boost::adjacency_list<
    boost::listS, boost::vecS, boost::directedS,
    Modelhierarchy, Connection>;

using TagHierarchyT = boost::labeled_graph<TagHierarchyGraph, std::string>;

using VertexT = boost::graph_traits<TagHierarchyGraph>::vertex_descriptor;
using EdgeT = boost::graph_traits<TagHierarchyGraph>::edge_descriptor;

class TagHierarchyImpl {
private:
    TagHierarchyGraph graph_;
    std::map<std::string, VertexT> vertices_;

public:
    bool PopulateGraph(std::vector<NodeType>& nodes) {
        for (auto & node : nodes) {
            std::string id = boost::get<std::string>(node["id"]);
            std::string parent_id = boost::get<std::string>(node["parent_id"]);
            std::cout << "Adding node " << id << " with parent id " << parent_id << std::endl;
            const auto current_vertex = boost::add_vertex({id, node}, graph_);
            vertices_[id] = current_vertex;
            const auto parent_vertex = vertices_.find(parent_id);
            if (parent_vertex != vertices_.end())
            {
                boost::add_edge(parent_vertex->second, current_vertex, Connection{"physical_hierarchy"}, graph_);
            }
        }
        for (auto vd : boost::make_iterator_range(boost::vertices(graph_))) {
            std::cout << graph_[vd].id << std::endl;
        }
        //boost::write_graphviz(std::cout, graph_,
        //                      [&](auto &out, auto v) {
        //                          out << "[id=\"" << graph_["b"].id << "\"]" << std::endl;
        //                          out << "[name=\"" << tag_hierarchy["b"].name << "\"]" << std::endl;
        //                          out << "[parent_id=\"" << tag_hierarchy["b"].parent_id << "\"]" << std::endl;
        //                          } /*,
        //                      [&](auto &out, auto e) {
        //                          out << "[type=\"" << tag_hierarchy["b"].type << "\"]";
        //                      }*/);
        return true;
    }
};

TagHierarchy::TagHierarchy() {
    impl_ = std::make_unique<TagHierarchyImpl>();
}

TagHierarchy::~TagHierarchy() {}

bool
TagHierarchy::PopulateGraph(std::vector<NodeType>& nodes) {
    std::cout << "Populating graph" << std::endl;
    return impl_->PopulateGraph(nodes);
}
