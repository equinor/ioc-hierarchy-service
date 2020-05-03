#include "models/models.h"

/* Teach boost test how to print a vector of NodeTypes. Needed to compile many test macros
 * involving vectors of NodeType
 * */
namespace boost {
    namespace test_tools {
        namespace tt_detail {
            template<>
            struct print_log_value<std::vector<NodeType>> {
            void operator()( std::ostream& os,
                             std::vector<NodeType> const& ts)
            {
                ::operator<<(os,ts);
            }
        };
    }
}
}

