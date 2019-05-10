#pragma once

#include <pybind11/pytypes.h>
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

namespace boost {
namespace serialization {

// Teach boost how to serialize pybind11::none
template<class Archive>
void serialize(Archive & ar, pybind11::none & g, const unsigned int version)
{
}

} // namespace serialization
} // namespace boost

using NodeType =
    std::map<std::string, boost::variant<int, double, std::string>>;

struct Modelhierarchy {
    std::string id;
    NodeType properties;
};

struct Connection {
    std::string type;
};
