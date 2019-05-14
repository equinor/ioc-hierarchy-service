#pragma once

#include "models/models.h"

#include <memory>

class TagHierarchyImpl;

class TagHierarchy {
public:
    TagHierarchy();
    TagHierarchy(const TagHierarchy& in);
    ~TagHierarchy(); 
    std::unique_ptr<TagHierarchyImpl> impl_;
    std::vector<NodeType> Handle(std::vector<NodeType>& message);
};
