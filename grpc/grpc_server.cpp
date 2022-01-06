#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <google/protobuf/map.h>
#include "hierarchy_service.grpc.pb.h"
#include "hierarchy_service.pb.h"

#include "tag_hierarchy/tag_hierarchy.h"
#include "grpc/type_conversion.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;


class HierarchyServiceImpl final : public HierarchyService::Service
{
  Status Query(ServerContext *context, const NodeList *query, NodeList *response)
  {
    auto local_query = std::vector<NodeType>();
    for (uint i = 0; i < query->node_size(); ++i) {
      auto node = query->node(i);
      local_query.push_back(TypeConversion::ConvertToLocalNode(node));
    }
    auto retval = TagHierarchy::Handle(local_query);
    for (auto iter = retval.cbegin(); iter < retval.cend(); ++iter) {
      Node* new_node = response->add_node();
      TypeConversion::ConvertToRemoteNode(*iter, new_node);
    }
    return Status::OK;
  }
  Status Store(ServerContext *context, const String *path, String* response) {
    auto result = TagHierarchy::Store(path->value());
    response->set_value(result);
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  HierarchyServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  builder.SetMaxMessageSize(25000000);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  // Expect only arg: --db_path=path/to/route_guide_db.json.
  RunServer();

  return 0;
}