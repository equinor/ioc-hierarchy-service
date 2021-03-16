//
// Created by Petter Moe Kvalvaag on 04/03/2020.
//

#ifndef TAG_HIERARCHY_SEARCH_H
#define TAG_HIERARCHY_SEARCH_H

#include <string>

namespace SearchUtils {
    struct Searcher {
        Searcher(const std::string& search_term) : search_term_(search_term) {
        }
        virtual bool HasTerm(std::string::const_iterator begin, std::string::const_iterator end) = 0;

        std::string search_term_;
    };
}
#endif //TAG_HIERARCHY_SEARCH_H
