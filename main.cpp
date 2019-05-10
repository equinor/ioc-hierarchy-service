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


//template < typename TimeMap > class bfs_time_visitor:public default_bfs_visitor {
//  typedef typename property_traits < TimeMap >::value_type T;
//public:
//  bfs_time_visitor(TimeMap tmap, T & t):m_timemap(tmap), m_time(t) { }
//  template < typename Vertex, typename Graph >
//    void discover_vertex(Vertex u, const Graph & g) const
//  {
//    put(m_timemap, u, m_time++);
//  }
//  TimeMap m_timemap;
//  T & m_time;
//};

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
        std::istringstream buffer(static_cast<char *>(request.data()));
        boost::archive::text_iarchive archive(buffer);
        std::vector<NodeType> test;

        archive >> test;

        tag_hierarchy.PopulateGraph(test);
        //for (auto const &dict : test)
        //{
        //    for (auto const &x : dict)
        //    {
        //        std::cout << x.first // string (key)
        //                  << ':'
        //                  << x.second // string's value
        //                  << std::endl;
        //    }
        //}
        //  Send reply back to client
        zmq::message_t reply(2);
        memcpy(reply.data(), "OK", 2);
        socket.send(reply);
    }
    //using namespace boost;
    // Select the graph type we wish to use

    //const auto v = Modelhierarchy{"a", "b", "c"};
    //vertex_t new_vertex = boost::add_vertex(v.id, tag_hierarchy);
    //tag_hierarchy["b"].name = v.name;
    //tag_hierarchy["b"].parent_id = v.parent_id;
    //vertex_t new_vertex2 = boost::add_vertex(Modelhierarchy{"d", "e", "f"}, tag_hierarchy);
    //auto new_edge = boost::add_edge(new_vertex, new_vertex2, Connection{"First connection"}, tag_hierarchy);
    //boost::write_graphviz(std::cout, tag_hierarchy,
    //                      [&](auto &out, auto v) {
    //                          out << "[id=\"" << tag_hierarchy["b"].id << "\"]" << std::endl;
    //                          out << "[name=\"" << tag_hierarchy["b"].name << "\"]" << std::endl;
    //                          out << "[parent_id=\"" << tag_hierarchy["b"].parent_id << "\"]" << std::endl;
    //                          } /*,
    //                      [&](auto &out, auto e) {
    //                          out << "[type=\"" << tag_hierarchy["b"].type << "\"]";
    //                      }*/);
    std::cout << std::flush;

    return 0;
}