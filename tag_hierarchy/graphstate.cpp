#include <tag_hierarchy/graphstate.h>

std::string s_serialized_graph;

void
GraphState::Store(const std::string& serialized_graph) {
    s_serialized_graph = serialized_graph;
}

std::string
GraphState::Retrieve() {
    return s_serialized_graph;
}
