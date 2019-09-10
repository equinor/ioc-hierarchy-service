FROM gcc:8 as cppbuild
# Install cmake
#RUN wget https://github.com/Kitware/CMake/releases/download/v3.14.3/cmake-3.14.3-Linux-x86_64.sh \
#      -q -O /tmp/cmake-install.sh \
#      && chmod u+x /tmp/cmake-install.sh \
#      && mkdir /usr/bin/cmake \
#      && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
#      && rm /tmp/cmake-install.sh
RUN update-alternatives --install /usr/bin/gfortran gfortran /usr/local/bin/gfortran 999 \
      && apt-get update && apt-get install -y cmake libboost-graph-dev libboost-serialization-dev \
         libboost-test-dev libpython3-dev libzmq3-dev

# ENV PATH="/usr/bin/cmake/bin:${PATH}"
WORKDIR /usr/src/app/
COPY . ioc-hierarchy-service
RUN mkdir ioc-hierarchy-service-docker-build
WORKDIR /usr/src/app/ioc-hierarchy-service-docker-build
RUN cmake ../ioc-hierarchy-service -DCMAKE_BUILD_TYPE=Release
RUN make -j6 && make install
RUN cpack .

