//
// Created by Petter Moe Kvalvaag on 2019-10-06.
//

#ifndef TAG_HIERARCHY_FILTEROPTIONS_H
#define TAG_HIERARCHY_FILTEROPTIONS_H

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/command.h"

class FilterOptions : Command {
public:
    FilterOptions();
    std::vector<NodeType> ProcessRequest(std::vector<NodeType>& request) override;
    std::string CommandName() override;
    DispatchFunction Function() override;
};

#endif //TAG_HIERARCHY_FILTEROPTIONS_H
