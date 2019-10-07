#pragma once

#include "models/models.h"

#include <memory>
#include <tag_hierarchy/commands/command.h>

class TagHierarchyImpl;

class TagHierarchy {
public:
    static TagHierarchyImpl& GetTagHierarchy();
    static std::vector<NodeType> Handle(std::vector<NodeType>& message);
    static TagHierarchyGraph& GetGraph();
    static std::map<std::string, VertexT>& GetVertices();
    static VertexT& GetRoot();
    static void Register(Command& in);
};
