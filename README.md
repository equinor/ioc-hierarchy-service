# ioc-hierarchy-service
In-memory queryable hierarchy service

This project generates a debian package with all components necessary to
create an in-memory cache of a hierarchy. The hierarchy is modelled as
a graph and can be queried in a number of ways. The cache itself is run
as a daemon process which can be backed up by using another daemon
process called backup service. All interprocess communication is handled
with ZeroMQ.

Querying the process must be done using the bundled python module. This
can be imported and a function named `query` can be run, e.g. to check
the health of the service:

```
>>> import tag_hierarchy_client
>>> tag_hierarchy_client.query([{'command': 'healthcheck'}], "server")
```

# Architectural overview
The following diagram describes which components are bundled with the
debian package and how they are intended to be used:

![Architecture diagram][docs/hierarchy_daemon.png]

## Communication
Components communicate via ZeroMQ(ZMQ). We use the patterns REQ-REP
(request-response) when the python client communicates with the server
or the statemanager.

From the python perspective, a request has to pass a list of dictionaries
and expects a list of dictionaries in return. By convention the first
dictionary of a request constitutes the command. Any number of
subsequent dictionaries constitute the data associated with the command.

The python module is built in C++ using the package `pybind11`, which
translates the python dictionaries into C++ native `std::map`. Since
C++ is statically typed, only a subset of dictionary key and value
types are permitted, and this is supported on the C++ by using
`boost::variant`. Currently the supported dictionary type is
`Dict[str, Union[bool, int, float, none, str, List[int], List[str]]]]`
Trying to pass objects that do not conform will raise a `TypeError`

## Tag hierarchy service
The tag hierarchy service keeps a tag hierarchy in memory. To populate
the tag hierarchy, use the `populate_hierarchy` command. To query the
hierarchy, the `nodes` command is used.

### Flow
Populating the hierarchy can take quite a bit of time, therefore the
package supports running in a statemanager mode which should not be used
for other purposes than managing the state of the hierarchy.

When the cache is updated, the population process happens against the
statemanager while the server is still serving requests. When the
population is done, it is asked to store itself via the backup service.
When the backup service has stored a new state, it notifies the server
via ZMQ PUB-SUB which the server will react to and it will get the
newest state from the backup service and initialize itself with that.
Finally the statemanager can clear its state so it does not consume
memory while idle.

## Backup service

### Storing the state
The backup service is a REQ-REP service. When it gets a message it will
try to store that message in redis under the key "hierarchy_cache" and
return `"Success"`. It will also PUBlish a message `"cache_updated"`
which the server should SUBscribe to and react to by retrieving the
latest state. 

### Retrieving the state
If the message sent to the backup service is `"GET_HIERARCHY"` it will
return a message with the serialized state, which the server can use to
re-initialize itself with the latest state.

TODO Add a command reference

# Development
## Getting started running CLion with Docker
Preconditions: Docker and CLion must be installed on your local computer

Follow instructions from step 3 onwards in the [stackoverflow article](https://stackoverflow.com/questions/55272484/how-to-code-run-programs-in-a-docker-container-using-clion/55424792) 
to configure clion. (The Dockerfile and docker-compose.yml mentioned
in the article are already in git.) Note however, if something goes wrong in step 3a, rebuild is not available from
the CLion. Commandline is necessary for doing docker-compose build to rebuild. If on windows running _Docker desktop_, you 
might have to go to Docker desktop settings and _expose localhost without TLS_.

