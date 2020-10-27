# This builds the a debian package of the project
FROM gcc:10 as cppbuild
RUN update-alternatives --install /usr/bin/gfortran gfortran /usr/local/bin/gfortran 999 \
      && apt-get update && apt-get install -y cmake zlib1g-dev libboost-graph-dev libtsan0 \
      libboost-serialization-dev libboost-test-dev libboost-iostreams-dev libpython3-dev libzmq3-dev

WORKDIR /usr/src/app/
COPY . ioc-hierarchy-service
RUN mkdir ioc-hierarchy-service-docker-build
WORKDIR /usr/src/app/ioc-hierarchy-service-docker-build
ARG CMAKE_BUILD_TYPE=Release
RUN cmake ../ioc-hierarchy-service -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
RUN make -j6 && make install
RUN cpack -G DEB .
WORKDIR /usr/src/app/ioc-hierarchy-service

