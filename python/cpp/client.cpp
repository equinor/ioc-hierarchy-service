#include "models/models.h"
#include "config/config.h"

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

namespace {
namespace local {
    std::shared_ptr<zmq::socket_t> get_socket (zmq::context_t& context, const std::string& app_mode) {
        auto socket = std::make_shared<zmq::socket_t>(context, ZMQ_REQ);
        const auto use_tcp = bool (std::getenv("ZEROMQ_USE_TCP"));
        if (app_mode == AppModeName.at(ApplicationMode::SERVER)) {
            const auto address = config::GetTagHierarchyServerAddress();
            socket->connect (address);
        }
        else {
            const auto address = config::GetTagHierarchyStatemanagerAddress();
            socket->connect (address);
        }
        const auto linger = int { 0 };
        socket->setsockopt(ZMQ_LINGER, &linger, sizeof (linger));
        return socket;
    }

    zmq::message_t
    SendMessage(std::shared_ptr<zmq::socket_t>& socket, std::vector<NodeType>& in) {
        auto buffer = std::ostringstream();
        {
            boost::archive::text_oarchive archive(buffer);
            archive << in;
        }
        zmq::message_t message((void*)buffer.str().c_str(), buffer.str().size()+1);
        socket->send(message);
        return message;
    }

    bool
    ProcessRequest(std::shared_ptr<zmq::socket_t> socket,
                   std::vector<NodeType>& reply_list,
                   int& retries_left,
                   zmq::message_t& message,
                   zmq::context_t& context,
                   const std::string& app_mode) {
        zmq::message_t reply;
        socket->recv(&reply);
        std::istringstream in_buffer(static_cast<char *>(reply.data()));
        boost::archive::text_iarchive archive(in_buffer);
        archive >> reply_list;
        socket->close();
        if (reply_list.size() > 0 &&
            reply_list.at(0).count("error") &&
            reply_list.at(0).count("action") &&
            boost::get<std::string>(reply_list.at(0).at("action")) == std::string("resend")) {
            if (--retries_left == 0) {
                reply_list.push_back({{{std::string("error"), std::string("daemon not responding")}}});
                return false;
            }
            std::cout << "Error: daemon could not deserialize message, retrying" << std::endl;
            std::cout << "The sent data was " << message.str() << std::endl;
            reply_list.clear();
            socket = local::get_socket(context, app_mode);
            socket->send(message);
        }
        else {
            return false;
        }
        return true;
    }
}
}

std::vector<NodeType> query(std::vector<NodeType> in,
                            std::string app_mode,
                            int request_timeout = 1500,
                            int request_retries = 2)
{
    zmq::context_t context (1);
    auto retries_left = int { request_retries };

    auto reply_list = std::vector<NodeType>();
    auto socket = local::get_socket(context, app_mode);

    zmq::message_t message = local::SendMessage(socket, in);

    auto expect_reply = bool { true };
    while (expect_reply){
        zmq::pollitem_t items[] = {
                {static_cast<void*>(*socket), 0 , ZMQ_POLLIN, 0}
        };
        zmq::poll(&items[0], 1, request_timeout);

            // If there is a reply, process it
        if (items[0].revents & ZMQ_POLLIN) {
            expect_reply &= local::ProcessRequest(socket, reply_list, retries_left,
                                                  message, context, app_mode);
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
            socket.reset();
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
        Supporting module for connection to the tag-hierarchy services
        -----------------------
        .. currentmodule:: tag_hierarchy_client
        .. autosummary::
           :toctree: _generate
           query
    )pbdoc";

    m.def("query", &query, R"pbdoc(
        Input a list of dictionaries to send to the server.
        The parameter 'app_mode' can be 'server' or 'statemanager'depending on who to talk to
        The parameter 'request_timeout' how long to wait for a reply in milliseconds
        The parameter 'request_retries' specifies how many times to retry on failure
    )pbdoc",
    py::arg("in"), py::arg("app_mode"),
    py::arg("request_timeout") = 300, py::arg("request_retries") = 3);

}