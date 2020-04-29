#pragma once

#include <pybind11/pytypes.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/variant.hpp>

#include <string>
#include <map>


// Typedefs for tag hierarchy
using Variants = boost::variant<
        bool,
        int,
        double,
        pybind11::none,
        std::string,
        std::vector<int>,
        std::vector<std::string>
>;

using NodeType = std::map<std::string, Variants>;

// How to print a NodeType
static inline std::ostream& operator << (std::ostream& os, const NodeType& node)
{
    os << std::endl << "    {" << std::endl;
    for (const auto& [key, value]: node) {
        if (value.type() == typeid(std::string)) {
            os << "        " << key << " : " << boost::get<std::string>(value) << std::endl;
        }
    }
    os << "    }" << std::endl;
    return os;
}

// How to print an std::vector of NodeTypes
static inline std::ostream& operator << (std::ostream& os, const std::vector<NodeType>& nodes)
{
    os << std::endl << "[" ;
    for (auto const node: nodes) {
        os << node;
    }
    os << "]" << std::endl;
    return os;
}

/*
 * Modelhierarchy is the struct which is associated with all vertices in the graph
 */
struct Modelhierarchy {
    std::string id;
    NodeType properties;
};

/*
 * Connection is the struct which is associated with all edges in the graph
 */
struct Connection {
    int id;
};

using EdgeLabelMap = std::map<std::string, int>;

using TagHierarchyGraph = boost::adjacency_list<
        boost::listS, boost::listS, boost::bidirectionalS,
        Modelhierarchy, Connection>;

using TagHierarchyT = boost::labeled_graph<TagHierarchyGraph, std::string>;

using VertexT = boost::graph_traits<TagHierarchyGraph>::vertex_descriptor;
using VertexIter = boost::graph_traits<TagHierarchyGraph>::vertex_iterator;
using EdgeT = boost::graph_traits<TagHierarchyGraph>::edge_descriptor;

using VertexDescMap = std::map<VertexT, size_t>;

namespace boost {
namespace serialization {

// Teach boost how to serialize pybind11::none
template<class Archive>
void serialize(Archive & ar, pybind11::none & g, const unsigned int version)
{
}

template<class Archive>
void serialize(Archive & ar, Modelhierarchy & g, const unsigned int version) {
    ar & g.id;
    ar & g.properties;
}

template<class Archive>
void serialize(Archive & ar, Connection & g, const unsigned int version) {
    ar & g.id;
}
} // namespace serialization
} // namespace boost

// Copied this ugly duckling from the pybind11 website. It is necessary
// for pybind11 to bind a boost::variant
// `boost::variant` as an example -- can be any `std::variant`-like container
namespace pybind11 { namespace detail {
    template <typename... Ts>
    struct type_caster<boost::variant<Ts...>> : variant_caster<boost::variant<Ts...>> {};

    // Specifies the function used to visit the variant -- `apply_visitor` instead of `visit`
    template <>
    struct visit_helper<boost::variant> {
        template <typename... Args>
        static auto call(Args &&...args) -> decltype(boost::apply_visitor(args...)) {
            return boost::apply_visitor(args...);
        }
    };
}} // namespace pybind11::detail

enum class ApplicationMode {STATE_MANAGER, SERVER};

static std::map<ApplicationMode, const char*> AppModeName = {
    {ApplicationMode::STATE_MANAGER, "statemanager"},
    {ApplicationMode::SERVER, "server"}
};
