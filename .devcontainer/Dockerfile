FROM public.ecr.aws/amazonlinux/amazonlinux:2023

RUN yum -y update && \
    yum -y groupinstall "Development Tools" && \
    yum -y install \
        make cmake git gcc gcc-c++ \
        openssl-devel ncurses-devel libcurl-devel libudev-devel \
        bison libtirpc-devel rpcgen gdb && \
    yum clean all

WORKDIR /workspace
