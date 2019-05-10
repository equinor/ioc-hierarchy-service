#pragma once

#include "models/models.h"

#include <memory>

class TagHierarchyImpl;

class TagHierarchy {
public:
    TagHierarchy();
    ~TagHierarchy(); 
    std::unique_ptr<TagHierarchyImpl> impl_;
    bool PopulateGraph(std::vector<NodeType>& nodes);
};
