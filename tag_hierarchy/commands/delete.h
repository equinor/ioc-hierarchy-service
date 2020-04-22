#pragma once

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/command.h"

class Delete : Command {
public:
    explicit Delete(std::string name);
private:
    std::vector<NodeType> ProcessRequest(std::vector<NodeType>& request) override;
};
