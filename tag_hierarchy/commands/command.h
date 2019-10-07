//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include <vector>
#include <models/models.h>
#include <tag_hierarchy/tag_hierarchy.h>
#include "tag_hierarchy/tag_hierarchy.h"

#ifndef TAG_HIERARCHY_COMMAND_H
#define TAG_HIERARCHY_COMMAND_H


using DispatchFunction =
std::function<std::vector<NodeType>(std::vector<NodeType>&)>;

class Command {
public:
    Command();
    std::vector<NodeType> Process(std::vector<NodeType>);
    virtual std::vector<NodeType> ProcessRequest(std::vector<NodeType>&) = 0;
    virtual std::string CommandName() = 0;
    virtual DispatchFunction Function() = 0;
};

#define REGISTER_COMMAND(Classname, Command) \
    Classname Classname##_instance;            \
    Classname::Classname() {TagHierarchy::Register(*this);} \
    std::string Classname::CommandName() {return #Command;}\
    DispatchFunction Classname::Function() {return std::bind(&Classname::ProcessRequest, *this, std::placeholders::_1);}


#endif //TAG_HIERARCHY_COMMAND_H
