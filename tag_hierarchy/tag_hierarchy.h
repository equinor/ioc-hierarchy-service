#pragma once

#include "models/models.h"

#include <memory>

class TagHierarchyImpl;

class TagHierarchy {
public:
    TagHierarchy();
    ~TagHierarchy(); 
    std::unique_ptr<TagHierarchyImpl> impl_;
    std::vector<NodeType> Handle(std::vector<NodeType>& message);
};
