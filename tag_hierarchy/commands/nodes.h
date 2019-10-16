//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#ifndef TAG_HIERARCHY_NODES_H
#define TAG_HIERARCHY_NODES_H

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/command.h"

class Nodes : Command {
public:
    Nodes();
    std::string CommandName() override;
    DispatchFunction Function() override;
private:
    std::vector<NodeType> ProcessRequest(std::vector<NodeType>& request) override;
};

#endif //TAG_HIERARCHY_NODES_H
