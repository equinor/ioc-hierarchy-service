#define BOOST_TEST_MODULE grpc_unit_test
#define BOOST_TEST_MAIN

#include "type_conversion.h"
#include "hierarchy_service.pb.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( GrpcUnitTest );
    BOOST_AUTO_TEST_CASE( test_roundtrip_proto_types_local_remote_local )
    {
        auto local = NodeType(
            {
                {std::string("name"),std::string("Level1-1-newname")},
                {std::string("id"),std::string("d9174b0d-2519-423f-c933-7491cce63858")},
                {std::string("parent_id"), pybind11::none()},
                {std::string("levelno"), 1 },
                {std::string("floating"), 1.5 },
                {std::string("is_modelelement"), false },
                {std::string("type"),std::string("folder")},
                {std::string("modelowner"), pybind11::none()},
                {std::string("vector_ints"), std::vector<int>{1, 2}},
                {std::string("vector_strings"), std::vector<std::string>{"test", "strings"}},
            }
        );
        auto remote_node = Node();
        TypeConversion::ConvertToRemoteNode(local, &remote_node);
        auto roundtripped_local = TypeConversion::ConvertToLocalNode(remote_node);
        BOOST_TEST(roundtripped_local == local);
    };

BOOST_AUTO_TEST_SUITE_END()
