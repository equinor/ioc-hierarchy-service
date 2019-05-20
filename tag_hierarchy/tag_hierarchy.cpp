#include "tag_hierarchy/tag_hierarchy.h"

#include "models/models.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>
#include <boost/graph/graph_utility.hpp> 

#include <functional>
#include <set>
#include <deque>
#include <typeinfo>

using TagHierarchyGraph = boost::adjacency_list<
    boost::listS, boost::vecS, boost::bidirectionalS,
    Modelhierarchy, Connection>;

using TagHierarchyT = boost::labeled_graph<TagHierarchyGraph, std::string>;

using VertexT = boost::graph_traits<TagHierarchyGraph>::vertex_descriptor;
using EdgeT = boost::graph_traits<TagHierarchyGraph>::edge_descriptor;
using VertexIterator = boost::graph_traits<TagHierarchyGraph>::vertex_iterator;
using InEdgeIterator = TagHierarchyGraph::in_edge_iterator;
using OutEdgeIterator = TagHierarchyGraph::out_edge_iterator;

using DispatchFunction =
    std::function<std::vector<NodeType>(std::vector<NodeType>&)>;


class FilteredHierarchyVisitor : public boost::default_dfs_visitor
{
public:
    explicit FilteredHierarchyVisitor(std::set<VertexT> &valid_nodes, std::map<VertexT, std::set<VertexT>>& valid_models,
                                      const std::vector<std::string> &kpifilter) : valid_nodes_(valid_nodes),
                                                                                   valid_models_(valid_models),
                                                                                   kpifilter_(kpifilter),
                                                                                   path_(std::deque<VertexT>()) {}

    void discover_vertex(VertexT v, const TagHierarchyGraph &g)
    {
        path_.push_front(v);
        if (g[v].properties.count("is_modelelement"))
        {
            const auto is_modelelement = boost::get<bool>(g[v].properties.find("is_modelelement")->second);
            if (is_modelelement)
            {
                if (kpifilter_.size() != 0 &&
                    g[v].properties.count("kpigroup_id"))
                {

                    if (g[v].properties.find("kpigroup_id")->second.type() == typeid(std::string))
                    {
                        const auto kpi_id = boost::get<std::string>(g[v].properties.find("kpigroup_id")->second);
                        if (std::find(std::cbegin(kpifilter_), std::cend(kpifilter_), kpi_id) == std::cend(kpifilter_))
                        {
                            return;
                        }
                    }
                    else { // KPI is null and filtered out
                        return;
                    }
                }
                for (auto const &path_part : path_)
                {
                    if (valid_nodes_.count(path_part) == 1)
                    {
                        break;
                    }
                    valid_nodes_.insert(path_part);
                }
                // I'm a valid model element. So the model needs to be added to the set of models
                // that pertain to the second level of the hierarchy I am traversing
                valid_models_[path_[path_.size() - 2]].insert(path_[1]);
            }
        }
        return;
    }
    void finish_vertex(VertexT v, const TagHierarchyGraph &g) {
        path_.pop_front();
    }

private:
    std::set<VertexT> &valid_nodes_;
    const std::vector<std::string> &kpifilter_;
    std::deque<VertexT> path_;
    std::map<VertexT, std::set<VertexT>>& valid_models_;
};

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
        nodes.erase(nodes.begin());
        root_ = boost::add_vertex({"root", {{"id", "root"}, {"levelno", 0}}}, graph_);
        for (auto &node : nodes)
        {
            std::string id = boost::get<std::string>(node["id"]);
            const auto current_vertex = boost::add_vertex({id, node}, graph_);
            std::cout << "Adding node " << id;
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
                    std::cout << " with parent id " << parent_id << std::endl;
                }
            }
            else
            {
                boost::add_edge(root_,
                                current_vertex,
                                Connection{"physical_hierarchy"},
                                graph_);
                std::cout << " with root as parentid " << std::endl;
            }

            vertices_[id] = current_vertex;
        }
        for (auto vd : boost::make_iterator_range(boost::vertices(graph_)))
        {
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
                              }*/
        );
        return retval;
    }

    std::vector<NodeType>
    Nodes(std::vector<NodeType> &nodes)
    {
        auto command_map = nodes.at(0);
        auto retval = std::vector<NodeType>();
        auto kpifilter = std::vector<std::string>();
        if (command_map.count("kpifilter") &&
            command_map["kpifilter"].type() == typeid(std::vector<std::string>))
        {
            kpifilter = boost::get<std::vector<std::string>>(command_map["kpifilter"]);
        }
        std::cout << "The valied kpis: ";
        for (auto kpifilterid : kpifilter) {
            std::cout << kpifilterid;
        }

        auto l2filter = std::vector<std::string>();
        if (command_map.count("l2filter") &&
            command_map["l2filter"].type() == typeid(std::vector<std::string>))
        {
            l2filter = boost::get<std::vector<std::string>>(command_map.at("l2filter"));
        }
        std::cout << "The l2filter items: ";
        for (auto l2filteritem : l2filter) {
            std::cout << l2filteritem;
        }

        std::cout << std::endl;
        auto valid_nodes = std::set<VertexT>();
        auto valid_models = std::map<VertexT, std::set<VertexT>>();
        auto dfs_visitor = FilteredHierarchyVisitor(valid_nodes, valid_models, kpifilter);

        auto const termfunc = [l2filter] (VertexT vertex, const TagHierarchyGraph& graph) {
            auto const levelno = boost::get<int>(graph[vertex].properties.at("levelno"));
            if (levelno == 2 && l2filter.size() > 0) {
                return std::find(cbegin(l2filter), cend(l2filter), graph[vertex].id) == cend(l2filter);
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
                         }){};
    TagHierarchyImpl(const TagHierarchyImpl& in) : command_func_dispatch_(in.command_func_dispatch_) {}
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
