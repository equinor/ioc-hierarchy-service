//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//
#pragma once

#include "tag_hierarchy/commands/command.h"

class CreateKPITree : Command {
public:
    CreateKPITree();
    std::string CommandName() override;
    DispatchFunction Function() override;
private:
    std::vector<NodeType> ProcessRequest(std::vector<NodeType>& request) override;
    std::vector<std::pair<std::string, std::string>> GetKPIGroups();
};


