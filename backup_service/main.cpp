//  file: main.cpp
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//

#include <zmq.hpp>

#include <iostream>

int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);

    std::cout << "Creating backup server" << std::endl;
    socket.bind ("tcp://127.0.0.1:5555");


    while (true) {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv(&request);

        // Convert the message to a vector of maps
        const auto message = std::string(static_cast<char*>(request.data()), request.size());


    }
    return 0;
}