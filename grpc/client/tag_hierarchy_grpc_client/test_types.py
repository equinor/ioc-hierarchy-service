from tag_hierarchy_types import convert_dict_to_proto, convert_proto_to_dict

def test_roundtrip_types():
    data = {
        'bool': False,
        'string': 'string',
        'int': 1,
        'float': 1.5,
        'null': None,
        'intlist': [1, 2],
        'stringlist': ['test', 'strings'],
    }
    proto = convert_dict_to_proto(data)
    roundtripped_data = convert_proto_to_dict(proto)
    assert roundtripped_data == data
