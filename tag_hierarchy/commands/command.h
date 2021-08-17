//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//
#pragma once

//#include "tag_hierarchy/tag_hierarchy.h"
#include <models/models.h>

#include <vector>


using DispatchFunction =
std::function<std::vector<NodeType>(std::vector<NodeType>&)>;

class Command {
public:
    explicit Command(std::string name);
    std::vector<NodeType> Process(std::vector<NodeType>& request);
    DispatchFunction Function();

protected:
    TagHierarchyGraph& GetGraph();
    std::unordered_map<std::string, VertexT>& GetVertices();
    VertexT& GetRoot();

private:
    virtual std::vector<NodeType> ProcessRequest(std::vector<NodeType>&) = 0;
    std::string name_;
};
