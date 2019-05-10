#include <models/models.h>

#include <zmq.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/variant.hpp>

#include <vector>
#include <algorithm>
#include <string>

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

int post_hierarchy_object(std::vector<NodeType> in)
{
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);

    std::cout << "Creating hierarchy server" << std::endl;
    socket.connect ("tcp://127.0.0.1:5556");

    //for (auto const &dict : in)
    //{
    //    for (auto const &x : dict)
    //    {
    //        std::cout << x.first // string (key)
    //                  << ':'
    //                  << x.second // string's value
    //                  << std::endl;
    //    }
    //}

    std::ostringstream buffer;
    {
        boost::archive::text_oarchive archive(buffer);
        archive << in;
    }
    // The result can be extracted from the stringstream
    std::cout << buffer.str() << std::endl;
    //zmq::message_t message(sizeof(buffer));
    zmq::message_t message((void*)buffer.str().c_str(), buffer.str().size()+1);
    //std::memcpy(message.data(), buffer.str().data(), buffer.str().length());
    socket.send(message);
    zmq::message_t reply;
    socket.recv(&reply);
    std::cout << "Received World " << reply << std::endl;
    socket.close();
    context.close();
    return 0;
}

std::vector<int>
aggregate_sparklines_list(std::vector<std::vector<int>> in)
{
    auto retval = std::vector<int>();
    const auto no_sparklines = in.size();
    if (no_sparklines == 0) {
        return retval;
    }
    auto lengths = std::vector<size_t>();
    lengths.reserve(in.size());
    size_t length = 338;
    //for (size_t i = 0; i < no_sparklines; ++i) {
    //    lengths.emplace_back(in[i].size());
    //    length = std::max(length, in[i].size());
    //}
    //const auto length = *std::max_element(lengths.cbegin(), lengths.cend());
    int max = -1;
    for (size_t i = 0; i < length; ++i)
    {
        for (size_t j = 0; j < no_sparklines; ++j)
        {
            if (in[j].size() > i) {
                max = std::max(max, in[j][i]);
            }
        }
        retval.push_back(max);
        max = -1;
    }
    return retval;
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

    m.def("aggregate_sparklines_int", &aggregate_sparklines_list, R"pbdoc(
        Input a python list of lists of integers of equal length,
        returns a list of the highest value at any single point,
        eg. [[1, 4, 2], [2, 1, 3]] -> [2, 4, 3]
    )pbdoc");

    m.def("aggregate_sparklines_np", &post_hierarchy_object, R"pbdoc(
        Input a python list of numpy arrays (int) of equal length,
        returns a list of the highest value at any single point,
        eg. [np.array([1, 4, 2]), np.array([2, 1, 3])] -> [2, 4, 3]
    )pbdoc");
}