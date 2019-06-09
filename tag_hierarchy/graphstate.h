#pragma once

#include <string>
#include <vector>

namespace GraphState { //TODO: stateless; make namespaced free functions
    void Store(const std::string& serialized_graph);
    std::string Retrieve();
}
