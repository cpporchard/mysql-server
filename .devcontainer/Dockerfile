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