//  file: main.cpp
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//

#include "tag_hierarchy/tag_hierarchy.h"

#include "models/models.h"

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

#include <iostream>

#include <zmq.hpp>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <iostream>


/* The messagehandler handles command which requires orchestration between
   the server and the hierarchy
*/
class MessageHandler {
    public:
    std::vector<NodeType>
    HandleRequest(std::vector<NodeType>& request) {
        NodeType command_map = request[0];
        std::string command = boost::get<std::string>(command_map["command"]);
        if (command == "store")
        {
            const auto reply = tag_hierarchy_.Handle(request);
            const auto serialized_hierarchy =
                boost::get<std::string>(reply[0].at("serialized_graph"));
            zmq::context_t context(1);
            zmq::socket_t socket(context, ZMQ_REQ);
            std::cout << "Connecting to backup service" << std::endl;
            socket.connect("tcp://127.0.0.1:5555");
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
            zmq::context_t context(1);
            zmq::socket_t socket(context, ZMQ_REQ);
            std::cout << "Connecting to backup service" << std::endl;
            socket.connect("tcp://127.0.0.1:5555");
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
                return tag_hierarchy_.Handle(request);
            }
            auto retval = std::vector<NodeType>();
            retval.push_back({{"error", "cache was not populated"}});
            return retval;
        }
        else {
            return tag_hierarchy_.Handle(request);
        }
    }


private:
    TagHierarchy tag_hierarchy_;
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
    auto mode = local::GetMode(argc, argv);
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);

    std::cout << "Creating hierarchy server" << std::endl;

    if (mode == ApplicationMode::SERVER) {
        socket.bind ("tcp://127.0.0.1:5556");
    }
    else {
        socket.bind ("tcp://127.0.0.1:5557");
    }

    auto messagehandler = MessageHandler();

    while (true) {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv(&request);

        // Convert the message to a vector of maps
        std::vector<NodeType> message;
        try
        {
            std::istringstream buffer(static_cast<char *>(request.data()));
            boost::archive::text_iarchive archive(buffer);

            archive >> message;
        }
        catch (const std::exception& exc) {
            std::cerr << exc.what() << std::endl;
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
            std::cout << out_buffer.str() << std::endl;
            //zmq::message_t message(sizeof(buffer));
            zmq::message_t reply((void *)out_buffer.str().c_str(), out_buffer.str().size() + 1);
            //std::memcpy(message.data(), buffer.str().data(), buffer.str().length());
            socket.send(reply);
        }
        catch (const std::exception& exc) {
            std::cerr << "Error when handling message: " << exc.what() << std::endl;
            zmq::message_t reply(5);
            memcpy(reply.data(), "Error", 5);
            socket.send(reply);
            continue;
        }

    }
    std::cout << std::flush;

    return 0;
}