//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//
#include "tag_hierarchy/tag_hierarchy.h"
#include <models/models.h>

#include <vector>

#ifndef TAG_HIERARCHY_COMMAND_H
#define TAG_HIERARCHY_COMMAND_H


using DispatchFunction =
std::function<std::vector<NodeType>(std::vector<NodeType>&)>;

class Command {
public:
    Command();
    std::vector<NodeType> Process(std::vector<NodeType>& request);
    virtual std::string CommandName() = 0;
    virtual DispatchFunction Function() = 0;

protected:
    TagHierarchyGraph& GetGraph();
    std::map<std::string, VertexT>& GetVertices();
    VertexT& GetRoot();
    void Register(Command& in);

private:
    virtual std::vector<NodeType> ProcessRequest(std::vector<NodeType>&) = 0;
};

#define REGISTER_COMMAND(Classname, CommandString) \
    Classname Classname##_instance;            \
    Classname::Classname() {Register(*this);} \
    std::string Classname::CommandName() {return #CommandString;}\
    DispatchFunction Classname::Function() {return [this](std::vector<NodeType>& in) {return this->Process(in);};}


#endif //TAG_HIERARCHY_COMMAND_H
