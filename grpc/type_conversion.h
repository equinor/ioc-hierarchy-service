#include "hierarchy_service.pb.h"

#include "models/models.h"


namespace TypeConversion {
  NodeType ConvertToLocalNode(Node proto_node);
  void ConvertToRemoteNode(const NodeType& local_node, Node* remote_node);
}
