# This builds the a debian package of the project
FROM debian:bullseye as cppbuild
RUN apt-get update && apt-get install -y zlib1g-dev libgflags-dev libboost-graph-dev libtsan0 \
      libboost-serialization-dev libboost-test-dev libboost-iostreams-dev libpython3-dev libzmq3-dev \
      libhiredis-dev pybind11-dev wget build-essential git python3 python3-distutils python3-pip file \
      curl zip unzip tar mono-complete
RUN wget https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3-Linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && /tmp/cmake-install.sh --skip-license --prefix=/usr/local \
      && rm /tmp/cmake-install.sh
# Download the latest stable `nuget.exe` to `/usr/local/bin`
RUN wget -o /usr/local/bin/nuget.exe https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
WORKDIR /usr/src/app/
RUN mkdir ioc-hierarchy-service
COPY vcpkg/ ioc-hierarchy-service/vcpkg/
RUN ioc-hierarchy-service/vcpkg/bootstrap-vcpkg.sh -disableMetrics



FROM cppbuild as generate_package
ARG FEED_ACCESSTOKEN
ARG FEED_URL
RUN curl -L https://raw.githubusercontent.com/Microsoft/artifacts-credprovider/master/helpers/installcredprovider.sh  | sh
ENV NUGET_CREDENTIALPROVIDER_SESSIONTOKENCACHE_ENABLED true
RUN mono /usr/local/bin/nuget.exe sources add -name "ADO" -Source "https://pkgs.dev.azure.com/equinorioc/_packaging/ioc-vcpkg/nuget/v3/index.json" -Username "docker" -Password "'${FEED_ACCESSTOKEN}'"
# TODO: do a "mono sources add -Name <somename> -Source <our package> -Username <username> -Password <access token>" 
#     Then: sett <somename> as the uri for VCPKG_BINARY_SOURCES
RUN d
ENV VCPKG_BINARY_SOURCES 'clear;nuget,{"endpointCredentials":[{"endpoint":"https://pkgs.dev.azure.com/equinorioc/_packaging/ioc-vcpkg/nuget/v3/index.json","username":"docker","password":"'${FEED_ACCESSTOKEN}'"}]},readwrite'
ENV VSS_NUGET_EXTERNAL_FEED_ENDPOINTS '{"endpointCredentials":[{"endpoint":"https://pkgs.dev.azure.com/equinorioc/_packaging/ioc-vcpkg/nuget/v3/index.json","username":"docker","password":"'${FEED_ACCESSTOKEN}'"}]}'
RUN echo $FEED_URL
RUN echo $VSS_NUGET_EXTERNAL_FEED_ENDPOINTS
RUN echo $FEED_ACCESSTOKEN
COPY . ioc-hierarchy-service
RUN pip install -r ioc-hierarchy-service/grpc/client/requirements.txt
RUN mkdir ioc-hierarchy-service-docker-build
COPY ./nuget.config .
WORKDIR /usr/src/app/ioc-hierarchy-service-docker-build
ARG CMAKE_BUILD_TYPE=Debug
RUN cmake ../ioc-hierarchy-service -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
RUN make -j6 && make install
# Build debian package
RUN cpack -G DEB .
# Build python wheel
RUN make -C /usr/src/app/ioc-hierarchy-service/grpc/client proto_python
RUN make -C /usr/src/app/ioc-hierarchy-service/grpc/client build_wheel
WORKDIR /usr/src/app/ioc-hierarchy-service
