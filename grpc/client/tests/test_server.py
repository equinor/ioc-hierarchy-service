import unittest
import json
from typing import List, Dict
from subprocess import Popen
from time import sleep

import grpc

from proto_build.hierarchy_service_pb2 import NodeList 
from proto_build.hierarchy_service_pb2_grpc import HierarchyServiceStub

from utils.types import protobuf_to_dict, convert_dict_to_proto


class TestServer(unittest.TestCase):
    def setUp(self) -> None:
        self._server = Popen(['../grpc_server'])
        channel = grpc.insecure_channel('127.0.0.1:50051')

        stub = HierarchyServiceStub(channel)
        with open('hierarchy_dump.json', 'r') as data:
            hierarchy: List[Dict] = json.load(data)

        query = NodeList()
        [query.node.append(convert_dict_to_proto(d)) for d in hierarchy]
        stub.Query(query)
        
        return super().setUp()

    def test_nodes(self):
        channel = grpc.insecure_channel('127.0.0.1:50051')
        stub = HierarchyServiceStub(channel)

        query = NodeList()
        command = {
            'command': 'nodes',
            'parentId': None,
        }
        query.node.append(convert_dict_to_proto(command))
        grpc_response = stub.Query(query)
        response = sorted(
            [protobuf_to_dict(a) for a in grpc_response.node],
            key=lambda x: x['name']
        )
        
        assert response[0]['name'] == 'Level1-1'
        assert response[1]['name'] == 'Level1-2'
        assert response[1]['levelno'] == 1
        assert response[1]['parent_id'] is None
        assert response[1]['is_modelelement'] is False
        assert response[1]['modelsource_id'] is None

        query = NodeList()
        command = {
            'command': 'nodes',
            'parentId': response[1]['id'],
        }
        query.node.append(convert_dict_to_proto(command))
        grpc_response = stub.Query(query)
        response_l2 = sorted(
            [protobuf_to_dict(a) for a in grpc_response.node],
            key=lambda x: x['name']
        )

        assert response_l2[0]['name'] == 'Level1-2->Level2-1'
        assert response_l2[1]['levelno'] == 2
        assert response_l2[1]['parent_id'] == response[1]['id']

        query = NodeList()
        command = {
            'command': 'nodes',
            'parentId': response_l2[0]['id'],
        }
        query.node.append(convert_dict_to_proto(command))
        grpc_response = stub.Query(query)
        response_l3 = sorted(
            [protobuf_to_dict(a) for a in grpc_response.node],
            key=lambda x: x['name']
        )

        assert response_l3[1]['name'] == 'Level1-2->Level2-1->Level3-2'
        assert response_l3[0]['levelno'] == 3
        assert response_l3[1]['parent_id'] == response_l2[0]['id']
        assert response_l3[0]['is_modelelement'] is False
        assert response_l3[0]['modelsource_id'] is not None

        query = NodeList()
        command = {
            'command': 'nodes',
            'parentId': response_l3[1]['id'],
        }
        query.node.append(convert_dict_to_proto(command))
        grpc_response = stub.Query(query)
        response_l4 = sorted(
            [protobuf_to_dict(a) for a in grpc_response.node],
            key=lambda x: x['name']
        )

        assert response_l4[0]['levelno'] == 4
        assert response_l4[1]['is_modelelement'] is True
        assert response_l4[0]['parent_id'] == response_l3[1]['id']

    def tearDown(self) -> None:
        self._server.terminate()
        self._server.wait()
        return super().tearDown()
