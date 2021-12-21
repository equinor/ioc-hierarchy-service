from hierarchy_service_pb2 import Node, Property, IntList, StringList

def convert_proto_to_dict(proto_node: Node):
    retval = {}
    for key in proto_node.map:
        value = proto_node.map[key]
        if value.HasField('boolean'):
            retval[key] = value.boolean
        elif value.HasField('integer'):
            retval[key] = value.integer
        elif value.HasField('floating'):
            retval[key] = value.floating
        elif value.HasField('stringval'):
            retval[key] = value.stringval
        elif value.HasField('intlist'):
            retval[key] = [i for i in value.intlist.values]
        elif value.HasField('stringlist'):
            retval[key] = value.stringlist.values
        else:
            retval[key] = None
        
    return retval


def convert_dict_to_proto(dict: dict):
    proto_node = Node()
    for key, item in dict.items():
        if (type(item)) == bool:
            proto_node.map[key].CopyFrom(Property(boolean=item))
        elif (type(item)) == int:
            proto_node.map[key].CopyFrom(Property(integer=item))
        elif (type(item)) == float:
            proto_node.map[key].CopyFrom(Property(floating=item))
        elif (type(item)) == str:
            proto_node.map[key].CopyFrom(Property(stringval=item))
        elif (item is None):
            proto_node.map[key].CopyFrom(Property())
        elif type(item) == list:
            if all(type(list_item) == int for list_item in item):
                int_list = IntList()
                int_list.values.extend(item)
                proto_node.map[key].CopyFrom(Property(intlist=int_list))
            elif all(type(list_item) == str for list_item in item):
                string_list = StringList()
                string_list.values.extend(item)
                proto_node.map[key].CopyFrom(Property(stringlist=string_list))

    return proto_node