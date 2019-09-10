//  file: main.cpp
//  
//  
//  
//

#include <zmq.hpp>

#include <cpp_redis/cpp_redis>

#include <iostream>
#include <sstream>

static const char* CACHE_KEY = "hierarchy_cache";

namespace local {
void GetRedisConnectionParams(std::string &redis_url,
                              std::size_t &redis_port,
                              int &redis_db,
                              std::string &redis_password)
{
    if (const char *redis_url_env = std::getenv("REDIS_URL_DAEMON"))
    {
        redis_url = std::string(redis_url_env);
    }
    std::stringstream str_value;
    str_value << std::getenv("REDIS_PORT");
    str_value >> redis_port;
    str_value.clear();
    str_value << std::getenv("REDIS_DB");
    str_value >> redis_db;
    str_value.clear();
    if (const char *redis_password_env = std::getenv("REDIS_PASSWORD"))
    {
        redis_password = std::string(redis_password_env);
    }
}
}

int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);

    std::cout << "Starting backup server" << std::endl;
    socket.bind ("tcp://127.0.0.1:5555");

    std::string redis_url;
    std::size_t redis_port;
    int redis_db;
    std::string redis_password;
    local::GetRedisConnectionParams(redis_url, redis_port, redis_db, redis_password);

    // Setup redis client
    cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);

    zmq::socket_t pub_socket (context, ZMQ_PUB);
    pub_socket.bind("tcp://127.0.0.1:5559");

    while (true)
    {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv(&request);

        // Convert the message to a std::string
        const auto message = std::string(static_cast<char *>(request.data()), request.size());
        auto reply_string = std::string();

        // Setup the redis client
        cpp_redis::client client;
        client.connect(redis_url, redis_port,
                       [](const std::string& host, std::size_t port, cpp_redis::connect_state status) {
                           if (status == cpp_redis::connect_state::dropped) {
                               std::cout << "Client disconnected from host " << host << std::endl;
                           }
                           else if (status == cpp_redis::connect_state::failed) {
                               std::cout << "Client failed to connect to " << host << std::endl;
                           }
                           else if (status == cpp_redis::connect_state::ok) {
                               std::cout << "Client connected successfully to " << host << std::endl;
                           }
                       });
        if (redis_password != "") {
            client.auth(redis_password);
        }
        client.select(redis_db);
        // Support setting or getting the hierarchy key
        // TODO: This whole service could/should be a generic redis connector!
        if (message == "GET_HIERARCHY")
        {
            std::cout << "Getting the cache key" << std::endl;
            auto reply_string_future = client.get(CACHE_KEY);
            client.sync_commit();
            try {
                reply_string = reply_string_future.get().as_string();
            }
            catch (cpp_redis::redis_error) { // Hierarchy is not cached
                reply_string = "ERROR";
            }
        }
        else
        {
            client.set(CACHE_KEY, message, [](const cpp_redis::reply &reply) {
                std::cout << "Set the cache" << std::endl;
            });
            client.sync_commit();
            reply_string = "Success";

            // Publish a message that the cache has been updated
            std::string pub_message("cache_updated");
            zmq::message_t pub(pub_message.size());
            memcpy(pub.data(), pub_message.c_str(), pub_message.size());
            pub_socket.send(pub);
        }

        client.disconnect();
        zmq::message_t reply(reply_string.size());

        memcpy(reply.data(), reply_string.c_str(), reply_string.size());
        socket.send(reply);
    }
    return 0;
}
