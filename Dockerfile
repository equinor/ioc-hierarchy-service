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
RUN ioc-hierarchy-service/vcpkg/bootstrap-vcpkg.sh -disableMetrics



FROM cppbuild as generate_package
ARG FEED_ACCESSTOKEN
ARG FEED_URL='https://pkgs.dev.azure.com/equinorioc/0adf653c-0d86-488b-bc00-d51fbe6e753d/_packaging/ioc-cpp-packages/nuget/v3/index.json'
RUN wget https://raw.githubusercontent.com/Microsoft/artifacts-credprovider/master/helpers/installcredprovider.sh \
      -q -O /tmp/installcredprovider.sh \
      && chmod u+x /tmp/installcredprovider.sh \
      && /tmp/installcredprovider.sh \
      && rm /tmp/installcredprovider.sh
ENV NUGET_CREDENTIALPROVIDER_SESSIONTOKENCACHE_ENABLED true
RUN if [ -n "$FEED_ACCESSTOKEN" ]; then \
      wget -q -O /usr/local/bin/nuget.exe https://dist.nuget.org/win-x86-commandline/latest/nuget.exe \
      && apt-get update && apt-get install -y mono-complete \
      && mono /usr/local/bin/nuget.exe sources add -name "ioc-cpp-packages" -Source ${FEED_URL} -Username "docker" -Password ${FEED_ACCESSTOKEN}; fi
ENV VCPKG_BINARY_SOURCES 'clear;nuget,ioc-cpp-packages,readwrite'
COPY . ioc-hierarchy-service
RUN pip install -r ioc-hierarchy-service/grpc/client/requirements.txt
RUN mkdir ioc-hierarchy-service-docker-build
WORKDIR /usr/src/app/ioc-hierarchy-service-docker-build
ARG CMAKE_BUILD_TYPE=Release
RUN cmake ../ioc-hierarchy-service -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
RUN make -j6 && make install
# Build debian package
RUN cpack -G DEB .
# Build python wheel
RUN make -C /usr/src/app/ioc-hierarchy-service/grpc/client proto_python
RUN make -C /usr/src/app/ioc-hierarchy-service/grpc/client build_wheel
WORKDIR /usr/src/app/ioc-hierarchy-service
