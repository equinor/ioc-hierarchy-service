import unittest
import json
import os, sys
from typing import List, Dict
from subprocess import Popen
from time import sleep

import grpc

from hierarchy_service_pb2 import NodeList, String
from hierarchy_service_pb2_grpc import HierarchyServiceStub

from tag_hierarchy_types import convert_proto_to_dict, convert_dict_to_proto


class TestServer(unittest.TestCase):
    def setUp(self) -> None:
        binary_dir = os.getenv('CMAKE_BINARY_DIR', '../../../build')
        self._server = Popen([os.path.join(binary_dir, 'grpc/grpc_server')])
        sleep(0.5)
        try:
            channel = grpc.insecure_channel('127.0.0.1:50051')

            stub = HierarchyServiceStub(channel)
            with open('../../../tag_hierarchy/unittests/hierarchy_dump.json', 'r') as data:
                hierarchy: List[Dict] = json.load(data)

            query = NodeList()
            [query.node.append(convert_dict_to_proto(d)) for d in hierarchy]
            stub.Query(query)
        except Exception as e:
            self._server.terminate()
            self._server.wait()
            raise e

        return super().setUp()

    def test_store_hierarchy(self):
        channel = grpc.insecure_channel('127.0.0.1:50051')
        stub = HierarchyServiceStub(channel)

        filename = os.path.join(os.getcwd(), 'hierarchy_snapshot.data')
        path = String(value=filename)
    
        grpc_response = stub.Store(path)
        assert os.path.isfile(filename)

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
            [convert_proto_to_dict(a) for a in grpc_response.node],
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
            [convert_proto_to_dict(a) for a in grpc_response.node],
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
            [convert_proto_to_dict(a) for a in grpc_response.node],
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
            [convert_proto_to_dict(a) for a in grpc_response.node],
            key=lambda x: x['name']
        )

        assert response_l4[0]['levelno'] == 4
        assert response_l4[1]['is_modelelement'] is True
        assert response_l4[0]['parent_id'] == response_l3[1]['id']

    def tearDown(self) -> None:
        self._server.terminate()
        self._server.wait()
        return super().tearDown()
