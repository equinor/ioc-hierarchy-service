# This builds the a debian package of the project
FROM debian:bullseye as cppbuild
RUN apt-get update && apt-get install -y zlib1g-dev libgflags-dev libboost-graph-dev libtsan0 \
      libboost-serialization-dev libboost-test-dev libboost-iostreams-dev libpython3-dev libzmq3-dev \
      libhiredis-dev pybind11-dev wget build-essential git python3 python3-distutils python3-pip file \
      curl zip unzip tar
RUN wget https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3-Linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && /tmp/cmake-install.sh --skip-license --prefix=/usr/local \
      && rm /tmp/cmake-install.sh
WORKDIR /usr/src/app/
RUN mkdir ioc-hierarchy-service
COPY vcpkg/ ioc-hierarchy-service/vcpkg/
RUN ioc-hierarchy-service/vcpkg/bootstrap-vcpkg.sh

FROM cppbuild as generate_package
COPY . ioc-hierarchy-service
RUN pip install -r ioc-hierarchy-service/grpc/client/requirements.txt
RUN mkdir ioc-hierarchy-service-docker-build
WORKDIR /usr/src/app/ioc-hierarchy-service-docker-build
ARG CMAKE_BUILD_TYPE=Release
RUN cmake ../ioc-hierarchy-service -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
RUN make -j6 && make install
RUN cpack -G DEB .
WORKDIR /usr/src/app/ioc-hierarchy-service
