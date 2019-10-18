//  file: main.cpp
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//

#include "tag_hierarchy/tag_hierarchy.h"

#include "models/models.h"
#include "config/config.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/range/irange.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/variant.hpp>

#include <opencensus/trace/span.h>
#include <opencensus/trace/with_span.h>
#include <opencensus/context/with_context.h>
#include <opencensus/exporters/trace/stdout/stdout_exporter.h>

#include <iostream>

#include <zmq.hpp>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <iostream>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
/* The messagehandler handles command which requires orchestration between
   the server and the hierarchy
*/
class MessageHandler {
public:
    MessageHandler() = delete;
    explicit MessageHandler(zmq::context_t& context) : context_(context) {};

    std::vector<NodeType>
    HandleRequest(std::vector<NodeType>& request) {
        if (request.empty()) {
            return {{{std::string("error"), std::string("no command passed")}}};
        }
        NodeType command_map = request[0];
        std::string command = boost::get<std::string>(command_map["command"]);
        const auto use_tcp = bool {std::getenv("ZEROMQ_USE_TCP")};
        if (command == "store")
        {
            const auto reply = TagHierarchy::Handle(request);
            const auto serialized_hierarchy =
                boost::get<std::string>(reply[0].at("serialized_graph"));
            zmq::socket_t socket(context_, ZMQ_REQ);
            std::cout << "Connecting to backup service" << std::endl;
            socket.connect(config::GetTagHierarchyBackupServiceAddress());
            zmq::message_t backup_request(serialized_hierarchy.size());
            memcpy(backup_request.data(), serialized_hierarchy.c_str(), serialized_hierarchy.size());
            socket.send(backup_request);
            zmq::message_t zmq_reply;
            socket.recv(&zmq_reply);
            auto retval = std::vector<NodeType>();
            retval.push_back({{"success", true}});
            return retval;
        }
        else if (command == "restore")
        {
            zmq::socket_t socket(context_, ZMQ_REQ);
            std::cout << "Connecting to backup service" << std::endl;
            socket.connect(config::GetTagHierarchyBackupServiceAddress());
            const auto message = std::string("GET_HIERARCHY");
            zmq::message_t backup_request(message.size());
            memcpy(backup_request.data(), message.c_str(), message.size());
            socket.send(backup_request);
            zmq::message_t zmq_reply;
            socket.recv(&zmq_reply);
            std::string backup_reply = std::string(static_cast<char *>(zmq_reply.data()),
                                                   zmq_reply.size());
            if (backup_reply != "ERROR") {
                request[0]["serialized_hierarchy"] = backup_reply;
                return TagHierarchy::Handle(request);
            }
            auto retval = std::vector<NodeType>();
            retval.push_back({{"error", "cache was not populated"}});
            return retval;
        }
        else {
            return TagHierarchy::Handle(request);
        }
    }
private:
    zmq::context_t& context_;
};

namespace {
    namespace local {
        /* Based on an applications input arguments, determine if we are running
           as a state manager or as a server */
        ApplicationMode GetMode(int argc, char* argv[]) {
            if (argc > 1) {
                if (strcmp(argv[1], AppModeName.at(ApplicationMode::SERVER)) == 0) {
                    return ApplicationMode::SERVER;
                }
                return ApplicationMode::STATE_MANAGER;
            }
            return ApplicationMode::SERVER;
        }
    }
}


int main (int argc, char* argv[])
{
    // Initialize stdout exporter for opencensus
    opencensus::exporters::trace::StdoutExporter::Register();
    auto mode = local::GetMode(argc, argv);
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);

    zmq::socket_t sub_socket (context, ZMQ_SUB);
    if (mode == ApplicationMode::SERVER) {
        std::cout << "Creating hierarchy server" << std::endl;
        // Listen to server requests
        socket.bind (config::GetTagHierarchyServerAddress());
        // Subscribe to updated state
        sub_socket.connect(config::GetTagHierarchyStateChangeAddress());
        sub_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    }
    else {
        std::cout << "Creating statemanager" << std::endl;
        socket.bind (config::GetTagHierarchyStatemanagerAddress());
    }
    zmq::pollitem_t items [] = {
        {static_cast<void*>(socket), 0, ZMQ_POLLIN, 0},
        {static_cast<void*>(sub_socket), 0, ZMQ_POLLIN, 0}
    };

    auto messagehandler = MessageHandler(context);

    static opencensus::trace::AlwaysSampler sampler;
    while (true) {
        const auto span_title = std::string("Receiving message for ") + AppModeName[mode];
        auto span = opencensus::trace::Span::StartSpan(span_title, nullptr, &sampler);
        zmq::message_t request;

        //  Wait for next request from client
        zmq::poll(&items[0], 2, -1);

        opencensus::trace::WithSpan ws(span);
        if (items[0].revents & ZMQ_POLLIN)
        {
            socket.recv(&request, ZMQ_DONTWAIT);
            // Convert the message to a vector of maps
            std::vector<NodeType> message;
            try
            {
                std::istringstream buffer(static_cast<char *>(request.data()));
                boost::archive::text_iarchive archive(buffer);
                archive >> message;
            }
            catch (const std::exception &exc)
            {
                const auto log_string = std::string("Command threw exception: ") + exc.what();
                span.AddAnnotation(log_string);
                std::ostringstream out_buffer;
                const auto reply_map =
                        std::vector<NodeType>({{{std::string("error"), std::string("deserializing stream failed")},
                                               {std::string("action"), std::string("resend")}}});
                {
                    boost::archive::text_oarchive archive(out_buffer);
                    archive << reply_map;
                }
                // The result can be extracted from the stringstream
                zmq::message_t reply((void *)out_buffer.str().c_str(), out_buffer.str().size() + 1);
                socket.send(reply);
            }

            try
            {
                const auto reply_list = messagehandler.HandleRequest(message);

                //  Send reply back to client
                std::ostringstream out_buffer;
                {
                    boost::archive::text_oarchive archive(out_buffer);
                    archive << reply_list;
                }
                // The result can be extracted from the stringstream
                zmq::message_t reply((void *)out_buffer.str().c_str(), out_buffer.str().size() + 1);
                socket.send(reply);
            }
            catch (const std::exception &exc)
            {
                const auto log_string = std::string("An error occurred while replying to command: ") + exc.what();
                span.AddAnnotation(log_string);
                std::ostringstream out_buffer;
                const auto reply_map =
                        std::vector<NodeType>({{{std::string("error"), std::string("command failed")},
                                               {std::string("action"), std::string("fix_parameters")}}});
                {
                    boost::archive::text_oarchive archive(out_buffer);
                    archive << reply_map;
                }
                // The result can be extracted from the stringstream
                zmq::message_t reply((void *)out_buffer.str().c_str(), out_buffer.str().size() + 1);
                socket.send(reply);
            }
        }
        if (items[1].revents & ZMQ_POLLIN) {
            sub_socket.recv(&request, ZMQ_DONTWAIT);
            const auto message = std::string(static_cast<char *>(request.data()), request.size());
            if (message == "cache_updated") {
                auto command = std::vector<NodeType>(
                    {{{"command", std::string("restore")}}}
                );
                messagehandler.HandleRequest(command);
            }
        }
        span.End();
    }

    return 0;
}
#pragma clang diagnostic pop