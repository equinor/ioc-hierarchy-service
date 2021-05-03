//  file: main.cpp
//  
//  
//  
//

#include "config/config.h"
#include <zmq.hpp>

#include <redis++.h>

#include <iostream>
#include <sstream>

static const char* CACHE_KEY = "hierarchy_cache";

namespace {
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
            str_value << std::getenv("REDIS_PORT_DAEMON");
            str_value >> redis_port;
            str_value.clear();
            str_value << std::getenv("REDIS_DB_DAEMON");
            str_value >> redis_db;
            str_value.clear();
            if (const char *redis_password_env = std::getenv("REDIS_PASSWORD_DAEMON"))
            {
                redis_password = std::string(redis_password_env);
            }
        }

        std::shared_ptr<sw::redis::Redis>
        GetRedisClient(const std::string& redis_url,
                       int redis_port,
                       const std::string& redis_password,
                       int redis_db) {
            sw::redis::ConnectionOptions connection_options;
            connection_options.host = redis_url;
            connection_options.port = redis_port;
            connection_options.password = redis_password;
            connection_options.db = redis_db;
            auto client = std::make_shared<sw::redis::Redis>(connection_options);
            return client;
        }

        void
        SetHierarchy(const std::string &message,
                     std::string &reply_string,
                     std::shared_ptr<sw::redis::Redis> &client) {
            client->set(CACHE_KEY, message);
            reply_string = "Success";
        }

        void
        GetHierarchy(std::string &reply_string,
                     std::shared_ptr<sw::redis::Redis> &client) {
            std::cout << "Getting the cache key" << std::endl;
            auto optional_reply_string = client->get(CACHE_KEY);
            if (optional_reply_string) {
                reply_string = *optional_reply_string;
            }
            else {
                reply_string = "ERROR";
            }
        }

        void EventLoop(zmq::socket_t &socket, const std::string &redis_url, size_t redis_port, int redis_db,
                       const std::string &redis_password, zmq::socket_t &pub_socket) {
            zmq::message_t request;

            //  Wait for next request from client
            socket.recv(&request);

            // Convert the message to a std::string
            const auto message = std::string(static_cast<char *>(request.data()), request.size());
            auto reply_string = std::string();

            // Setup the redis client
            auto client = GetRedisClient(redis_url, redis_port, redis_password, redis_db);
            // Set or get the hierarchy key
            if (message == "GET_HIERARCHY") {
                GetHierarchy(reply_string, client);
            }
            else {
                SetHierarchy(message, reply_string, client);

                // Publish a message that the cache has been updated
                std::string pub_message("cache_updated");
                zmq::message_t pub(pub_message.size());
                memcpy(pub.data(), pub_message.c_str(), pub_message.size());
                pub_socket.send(pub);
            }
            zmq::message_t reply(reply_string.size());
            memcpy(reply.data(), reply_string.c_str(), reply_string.size());
            socket.send(reply);
        }
    }
}

int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);

    std::cout << "Starting backup server" << std::endl;
    socket.bind (config::GetTagHierarchyBackupServiceAddress());

    // Set up Redis client
    std::string redis_url;
    std::size_t redis_port;
    int redis_db;
    std::string redis_password;
    local::GetRedisConnectionParams(redis_url, redis_port, redis_db, redis_password);

    zmq::socket_t pub_socket (context, ZMQ_PUB);
    pub_socket.bind(config::GetTagHierarchyStateChangeAddress());

    while (true)
    {
        local::EventLoop(socket, redis_url, redis_port, redis_db, redis_password, pub_socket);
    }
    return 0;
}

