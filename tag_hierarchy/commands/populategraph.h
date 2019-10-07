//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#ifndef TAG_HIERARCHY_POPULATEGRAPH_H
#define TAG_HIERARCHY_POPULATEGRAPH_H

#endif //TAG_HIERARCHY_POPULATEGRAPH_H

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/command.h"

class PopulateGraph : Command {
public:
    PopulateGraph();
    std::vector<NodeType> ProcessRequest(std::vector<NodeType>& request) override;
    std::string CommandName() override;
    DispatchFunction Function() override;
};


