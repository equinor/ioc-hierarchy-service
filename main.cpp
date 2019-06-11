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


int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);

    std::cout << "Creating hierarchy server" << std::endl;
    socket.bind ("tcp://127.0.0.1:5556");

    auto tag_hierarchy = TagHierarchy();

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
            const auto reply_list = tag_hierarchy.Handle(message);

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