//
// Created by Petter Moe Kvalvaag on 15/10/2019.
//

#ifndef TAG_HIERARCHY_CONFIG_H
#define TAG_HIERARCHY_CONFIG_H

#include <cstring>
#include <cstdlib>

namespace config {
    bool IsUseTcp() {
        static bool is_use_tcp = std::getenv("ZEROMQ_USE_TCP") && strcmp(std::getenv("ZEROMQ_USE_TCP"), "1");
        return is_use_tcp;
    }
    const char* GetTagHierarchyServerAddress() {
        static const char* tag_hierarchy_server_address =
                IsUseTcp() ?
                "tcp://127.0.0.1:5556" : "ipc:///tag.hierarchy.server";
        return tag_hierarchy_server_address;
    }
    const char* GetTagHierarchyStatemanagerAddress() {
        static const char* tag_hierarchy_server_address =
                IsUseTcp() ?
                "tcp://127.0.0.1:5557" : "ipc:///tag.hierarchy.statemanager";
        return tag_hierarchy_server_address;
    }
    const char* GetTagHierarchyBackupServiceAddress() {
        static const char* tag_hierarchy_server_address =
                IsUseTcp() ?
                "tcp://127.0.0.1:5555" : "ipc:///tag.hierarchy.backup";
        return tag_hierarchy_server_address;
    }
    const char* GetTagHierarchyStateChangeAddress() {
        static const char* tag_hierarchy_server_address =
                IsUseTcp() ?
                "tcp://127.0.0.1:5559" : "ipc:///tag.hierarchy.statechange";
        return tag_hierarchy_server_address;
    }
}

#endif //TAG_HIERARCHY_CONFIG_H
