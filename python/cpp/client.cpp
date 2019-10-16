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

#define REQUEST_TIMEOUT 1500
#define REQUEST_RETRIES 3

namespace {
namespace local {
    std::shared_ptr<zmq::socket_t> get_socket (zmq::context_t& context, const std::string& app_mode) {
        auto socket = std::make_shared<zmq::socket_t>(context, ZMQ_REQ);
        if (app_mode == AppModeName.at(ApplicationMode::SERVER)) {
            socket->connect ("tcp://127.0.0.1:5556");
        }
        else {
            socket->connect ("tcp://127.0.0.1:5557");
        }
        const auto linger = int { 0 };
        socket->setsockopt(ZMQ_LINGER, &linger, sizeof (linger));
        return socket;
    }
}
}

std::vector<NodeType> query(std::vector<NodeType> in,
                            std::string app_mode,
                            int request_timeout = 1500,
                            int request_retries = 1)
{
    zmq::context_t context (1);
    auto retries_left = int { request_retries };

    auto reply_list = std::vector<NodeType>();
    auto socket = local::get_socket(context, app_mode);

    auto buffer = std::ostringstream();
    {
        boost::archive::text_oarchive archive(buffer);
        archive << in;
    }
    zmq::message_t message((void*)buffer.str().c_str(), buffer.str().size()+1);

    socket->send(message);

    auto expect_reply = bool { true };
    while (expect_reply){
        zmq::pollitem_t items[] = {
                {static_cast<void*>(*socket), 0 , ZMQ_POLLIN, 0}
        };
        zmq::poll(&items[0], 1, request_timeout);

        // If there is a reply, process it
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t reply;
            socket->recv(&reply);
            std::istringstream in_buffer(static_cast<char *>(reply.data()));
            boost::archive::text_iarchive archive(in_buffer);
            archive >> reply_list;
            socket->close();
            if (reply_list.at(0).count("error") &&
                reply_list.at(0).count("action") &&
                boost::get<std::string>(reply_list.at(0).at("action")) == std::string("resend")) {
                reply_list.clear();
                expect_reply = true;
            }
            else {
                expect_reply = false;
            }
        }
        else if (--retries_left == 0) {
            std::cout << "Error: hierarchy daemon does not reply" << std::endl;
            expect_reply = false;
            socket->close();
            socket.reset();
            reply_list.push_back({{{std::string("error"), std::string("daemon not responding")}}});
        }
        else {
            std::cout << "Warning: no response from daemon, retrying" << std::endl;
            socket->close();
            socket = local::get_socket(context, app_mode);
            socket->send(message);
        }

    }
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
    )pbdoc",
    py::arg("in"), py::arg("app_mode"),
    py::arg("request_timeout") = 300, py::arg("request_retries") = 3);

}