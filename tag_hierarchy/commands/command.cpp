//
// Created by Petter Moe Kvalvaag on 2019-10-03.
//

#include "tag_hierarchy/commands/command.h"
#include "tag_hierarchy/tag_hierarchy.h"

#include <opencensus/trace/span.h>
#include <opencensus/trace/with_span.h>
#include <opencensus/context/with_context.h>

Command::Command() {

}

std::vector<NodeType>
Command::Process(std::vector<NodeType>& request) {
    static opencensus::trace::AlwaysSampler sampler;
    auto result = std::vector<NodeType>();
    auto span = opencensus::trace::Span::StartSpan(CommandName(), nullptr, &sampler);
    span.AddAnnotation("Start running command " + CommandName());
    {
        opencensus::trace::WithSpan ws(span);
        try {
            result = ProcessRequest(request);
            span.AddAnnotation("Finished running command");
            if (!result.empty() &&
                result.at(0).count("error")) {
                const auto log_string = std::string("The ") + CommandName() +
                                        std::string(" command failed with the error ") +
                                        boost::get<std::string>(result.at(0).at("error"));
                span.AddAnnotation(log_string);
            }
        }
        catch (std::exception e) {
            const auto log_string = std::string("Command threw exception: ") + e.what();
            span.AddAnnotation(log_string);
        }
    }
    span.End();
    return result;
}

TagHierarchyGraph &Command::GetGraph() {
    return TagHierarchy::GetGraph();
}

std::map<std::string, VertexT> &Command::GetVertices() {
    return TagHierarchy::GetVertices();
}

VertexT &Command::GetRoot() {
    return TagHierarchy::GetRoot();
}

void Command::Register(Command &in) {
    TagHierarchy::Register(in);
}
