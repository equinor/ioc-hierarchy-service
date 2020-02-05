#include "models/models.h"

#include "boost/graph/reverse_graph.hpp"

namespace TagHierarchyUtils {
    namespace Graph {
        template<typename GraphT>
        int FindLevel(VertexT vertex, const GraphT& g) { // assuming there's only 1
            int level = 0;
            const auto reverse_graph = boost::reverse_graph<GraphT>(g);
            while (vertex != 0) {
                ++level;
                auto [start, end] = boost::adjacent_vertices(vertex, reverse_graph);
                if (start != end) {
                    vertex = *start;
                }
                else {
                    break;
                }
            }
            return level;
        }
    }
}
