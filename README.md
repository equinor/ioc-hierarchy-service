# ioc-hierarchy-service
In-memory queryable hierarchy service

# Getting started
Preferred IDE: Clion from jetbrains.
As can be seen in the dockerfile, the following dependencies must be installed:
* boost libraries
* zmq libraries
## On a PC
* Install [C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) and in CLion|File|Settings configure
toolchain to be Visual studio, architecture Amd64.
* To install boost, go to [boost download website](https://www.boost.org/users/download/) 
and select the _prebuilt windows binaries_, a little down the page. It gives you an installation program that copies
the library to your selected destination. In CLion|File|Settings configure BOOST_ROOT= directory where boost is installed.
* To install zmq, go to [zmq download website](https://zeromq.org/download/) and find the zmq prebuilt libraries, for 
visual studio. Extract to your selected destiantion. 

HOW make cmake find zmq???

  
  

