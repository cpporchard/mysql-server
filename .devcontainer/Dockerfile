FROM public.ecr.aws/amazonlinux/amazonlinux:2023

RUN dnf -y update
RUN dnf -y groupinstall "Development Tools"

# Install packages using one line each for clear logs
RUN dnf -y install make
RUN dnf -y install cmake
RUN dnf -y install git
RUN dnf -y install gcc
RUN dnf -y install gcc-c++
RUN dnf -y install openssl-devel
RUN dnf -y install ncurses-devel
RUN dnf -y install libcurl-devel
RUN dnf -y install libudev-devel
RUN dnf -y install bison
RUN dnf -y install libtirpc-devel
RUN dnf -y install rpcgen
RUN dnf -y install gdb

# Optional CLI tools
RUN dnf -y install vim
RUN dnf -y install procps
RUN dnf -y install lsof

RUN dnf clean all

RUN mkdir -p build && cd build && \
    cmake \
      -DCMAKE_C_COMPILER=/usr/bin/gcc \
      -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_FLAGS="-Wno-error=unused-variable" \
      -DDOWNLOAD_BOOST=1 \
      -DWITH_BOOST=/home/ec2-user/boost_1_77_0 \
      -DWITH_BUILD_ID=OFF \
      -DWITH_CLIENT_PROTOCOL_TRACING=OFF \
      -DWITH_FEDERATED_STORAGE_ENGINE=OFF \
      -DWITH_INNODB_MEMCACHED=ON \
      -DWITH_NDBCLUSTER_STORAGE_ENGINE=OFF \
      -DWITH_ROUTER=OFF \
      -DCMAKE_INSTALL_PREFIX=/usr \
      .. && \
    make -j$(nproc)