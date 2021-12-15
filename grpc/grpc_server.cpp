#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <google/protobuf/map.h>
#include "hierarchy_service.grpc.pb.h"
#include "hierarchy_service.pb.h"

#include "tag_hierarchy/tag_hierarchy.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

namespace {
  namespace local {
    NodeType ConvertToLocalNode(Node proto_node) {
      NodeType retval;
      const auto map = proto_node.map();
      for (auto iter = map.cbegin(); iter != map.cend(); ++iter) {
        const auto key = iter->first;
        const auto value = iter->second;
        const auto type = value.value_case();
        switch (type) {
          case Property::kBoolean:
            retval[key] = boost::variant<bool>(value.boolean());
            break;
          case Property::kInteger:
            retval[key] = boost::variant<int>(value.integer());
            break;
          case Property::kFloating:
            retval[key] = boost::variant<double>(value.floating());
            break;
          case Property::kStringval:
            retval[key] = boost::variant<std::string>(value.stringval());
            break;
          case Property::kIntlist:
          {
            auto int_vector = std::vector<int>();
            for (int i = 0; i < value.intlist().values_size(); ++i)
            {
              int_vector.push_back(value.intlist().values(i));
            }
            retval[key] = boost::variant<std::vector<int>>(int_vector);
            break;
          }
          case Property::kStringlist:
          {
            auto string_vector = std::vector<std::string>();
            for (int i = 0; i < value.stringlist().values_size(); ++i)
            {
              string_vector.push_back(value.stringlist().values(i));
            }
            retval[key] = boost::variant<std::vector<std::string>>(string_vector);
            break;
          }
          break;
          default:
            retval[key] = boost::variant<pybind11::none>();
            break;
        }
      }
      return retval;
    }
    Node ConvertToRemoteNode(const NodeType& local_node, Node* remote_node) {
      auto retval = Node();
      for (auto iter = local_node.cbegin(); iter != local_node.cend(); ++iter) {
        auto property = Property();
        if (iter->second.type() == typeid(bool)) {
          property.set_boolean(boost::get<bool>(iter->second));
        }
        if (iter->second.type() == typeid(int)) {
          property.set_integer(boost::get<int>(iter->second));
        }
        if (iter->second.type() == typeid(double)) {
          property.set_floating(boost::get<double>(iter->second));
        }
        if (iter->second.type() == typeid(std::string)) {
          property.set_stringval(boost::get<std::string>(iter->second));
        }
        if (iter->second.type() == typeid(std::vector<int>)) {
          const auto local_list = boost::get<std::vector<int>>(iter->second);
          auto int_list = property.mutable_intlist();
          for (int i = 0; i < local_list.size(); ++i) {
            int_list->add_values(local_list.at(i));
          }
        }
        if (iter->second.type() == typeid(std::vector<std::string>)) {
          const auto local_list = boost::get<std::vector<std::string>>(iter->second);
          auto string_list = property.mutable_stringlist();
          for (int i = 0; i < local_list.size(); ++i) {
            string_list->add_values(local_list.at(i));
          }
        }
        remote_node->mutable_map()->insert({iter->first, property});
      }
      return retval;
    }
  }
}
class HierarchyServiceImpl final : public HierarchyService::Service
{
  Status Query(ServerContext *context, const NodeList *query, NodeList *response)
  {
    auto local_query = std::vector<NodeType>();
    for (uint i = 0; i < query->node_size(); ++i) {
      auto node = query->node(i);
      local_query.push_back(local::ConvertToLocalNode(node));
    }
    auto retval = TagHierarchy::Handle(local_query);
    for (auto iter = retval.cbegin(); iter < retval.cend(); ++iter) {
      Node* new_node = response->add_node();
      auto remote_node = local::ConvertToRemoteNode(*iter, new_node);
    }
    return Status::OK;
  }

  // public:
  // HierarchyServiceImpl() {
    // auto mode = local::GetMode(argc, argv);

    //  Prepare our context and sockets
    // auto messagehandler = MessageHandler(context);

    // while (true)
    // {
    //   zmq::message_t request;

    //   //  Wait for next request from client
    //   zmq::poll(&items[0], 2, -1);

    //   if (items[0].revents & ZMQ_POLLIN)
    //   {
    //     local::HandleReqRep(socket, request, messagehandler);
    //   }
    //   if (items[1].revents & ZMQ_POLLIN)
    //   {
    //     local::HandleSubscribe(sub_socket, request, messagehandler);
    //   }
    // }

  // }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  HierarchyServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  // Expect only arg: --db_path=path/to/route_guide_db.json.
  RunServer();

  return 0;
}