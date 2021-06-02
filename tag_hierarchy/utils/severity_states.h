//
// Created by Erik Kvam Måland on 02/06/2021.
//

#ifndef TAG_HIERARCHY_SEVERITY_STATES_H
#define TAG_HIERARCHY_SEVERITY_STATES_H

static std::map<std::string, int> state_severity = {
        {"RE", 0},
        {"CO", 1},
        {"NO", 2},
        {"OW", 3},
        {"CR", 4},
};

int get_severity_level(std::string state) {
    return state_severity[state];
}

std::string get_state(int severity_level) {
    for (auto iterator = state_severity.begin(); iterator =! state_severity.end(); iterator++) {
        if (iterator->second == severity_level) {
            return iterator->first;
        }
    }
    return null;
}

#endif //TAG_HIERARCHY_SEVERITY_STATES_H
