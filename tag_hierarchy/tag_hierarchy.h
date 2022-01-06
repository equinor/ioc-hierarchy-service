#pragma once

#include "models/models.h"

#include <memory>
#include <tag_hierarchy/commands/command.h>

class TagHierarchyImpl;

class TagHierarchy {
    friend class Command;
public:
    static TagHierarchyImpl& GetTagHierarchy();
    static std::vector<NodeType> Handle(std::vector<NodeType>& message);
    static std::string Store(const std::string& path);
    static std::string Restore(const std::string& path);
    static std::string Clear();
    static std::pair<std::string, std::string> HealthCheck();

private:
    static TagHierarchyGraph& GetGraph();
    static std::unordered_map<std::string, VertexT>& GetVertices();
    static EdgeLabelMap & GetEdgeLabels();
    static VertexT& GetRoot();
    static void Register(Command& in, std::string name);
};
