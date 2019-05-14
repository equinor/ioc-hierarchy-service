#include "models/models.h"

#include <zmq.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/variant.hpp>

#include <vector>
#include <algorithm>
#include <string>

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

std::vector<NodeType> query(std::vector<NodeType> in)
{
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);

    std::cout << "Creating hierarchy server" << std::endl;
    socket.connect ("tcp://127.0.0.1:5556");

    std::ostringstream buffer;
    {
        boost::archive::text_oarchive archive(buffer);
        archive << in;
    }
    zmq::message_t message((void*)buffer.str().c_str(), buffer.str().size()+1);
    socket.send(message);
    zmq::message_t reply;
    socket.recv(&reply);
    std::istringstream in_buffer(static_cast<char *>(reply.data()));
    boost::archive::text_iarchive archive(in_buffer);
    std::vector<NodeType> reply_list;
    archive >> reply_list;
    socket.close();
    context.close();
    return reply_list;
}

namespace py = pybind11;

PYBIND11_MODULE(tag_hierarchy_client, m)
{
    m.doc() = R"pbdoc(
        Supporting module for performant aggregation
        of status vectors
        -----------------------
        .. currentmodule:: aggregate_sparklines
        .. autosummary::
           :toctree: _generate
           aggregate_sparklines_np_array
           aggregate_sparklines_list
    )pbdoc";

    m.def("query", &query, R"pbdoc(
        Input a python list of lists of integers of equal length,
        returns a list of the highest value at any single point,
        eg. [[1, 4, 2], [2, 1, 3]] -> [2, 4, 3]
    )pbdoc");

    m.def("aggregate_sparklines_np", &query, R"pbdoc(
        Input a python list of numpy arrays (int) of equal length,
        returns a list of the highest value at any single point,
        eg. [np.array([1, 4, 2]), np.array([2, 1, 3])] -> [2, 4, 3]
    )pbdoc");
}