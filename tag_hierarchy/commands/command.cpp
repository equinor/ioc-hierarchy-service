//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include "tag_hierarchy/commands/command.h"
#include "tag_hierarchy/tag_hierarchy.h"

Command::Command(std::string name) : name_(name) {
    TagHierarchy::Register(*this, name);
}

std::vector<NodeType>
Command::Process(std::vector<NodeType>& request) {
    auto result = std::vector<NodeType>();
    {
        try {
            result = ProcessRequest(request);
            if (!result.empty() &&
                result.at(0).count("error")) {
                const auto log_string = std::string("The ") + name_ +
                                        std::string(" command failed with the error ") +
                                        boost::get<std::string>(result.at(0).at("error"));
                std::cout << log_string << std::endl;
            }
        }
        catch (std::exception e) {
            const auto log_string = std::string("Command threw exception: ") + e.what();
            std::cout << log_string << std::endl;
        }
    }
    return result;
}

DispatchFunction Command::Function() {
    return [this](std::vector<NodeType>& in) {return this->Process(in);};
}

TagHierarchyGraph &Command::GetGraph() {
    return TagHierarchy::GetGraph();
}

std::unordered_map<std::string, VertexT> &Command::GetVertices() {
    return TagHierarchy::GetVertices();
}

VertexT &Command::GetRoot() {
    return TagHierarchy::GetRoot();
}
