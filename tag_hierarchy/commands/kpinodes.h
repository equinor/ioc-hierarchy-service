//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//
#pragma once

#include "tag_hierarchy/tag_hierarchy.h"
#include "tag_hierarchy/commands/command.h"

class KpiNodes : Command {
public:
    KpiNodes();
    std::string CommandName() override;
    DispatchFunction Function() override;
private:
    std::vector<NodeType> ProcessRequest(std::vector<NodeType>& request) override;
};
