# ioc-hierarchy-service
In-memory queryable hierarchy service
# Architectural overview
TODO insert data flow diagram with various components

TODO insert diagram showing deployment

# Development
## Getting started running CLion with Docker
Preconditions: Docker and CLion must be installed on your local computer

Follow instructions from step 3 onwards in the [stackoverflow article](https://stackoverflow.com/questions/55272484/how-to-code-run-programs-in-a-docker-container-using-clion/55424792) 
to configure clion. (The Dockerfile and docker-compose.yml mentioned
in the article are already in git.) Note however, if something goes wrong in step 3a, rebuild is not available from
the CLion. Commandline is necessary for doing docker-compose build to rebuild. If on windows running _Docker desktop_, you 
might have to go to Docker desktop settings and _expose localhost without TLS_.

